#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>

#include "config.h"
#include "types.h"
#include "loader.h"
#include "cluster.h"
#include "cost.h"
#include "results.h"
#include "distance.h"
#include "timing.h"

static void print_usage(const char *prog) {
    fprintf(stderr, "Help: %s -i <dataset> -s <solution> -o <results> [options]\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -k INT    Number of clusters to form (default: %d)\n", DEFAULT_NUM_CLUSTERS);
    fprintf(stderr, "  -l INT    Max time to complete a route per cluster (default: %d)\n", DEFAULT_MAX_LOAD_SECONDS);
    fprintf(stderr, "  -p {0,1}  Active penalization in the objective function (default: 1)\n");
    fprintf(stderr, "  -h        Show this help \n");
}

static int parse_args(int argc, char **argv, ProgramArgs *args) {
    static struct option long_opts[] = {
        {"dataset",      required_argument, 0, 'i'},
        {"solution",     required_argument, 0, 's'},
        {"results",      required_argument, 0, 'o'},
        {"num-clusters", required_argument, 0, 'k'},
        {"max-load",     required_argument, 0, 'l'},
        {"penalization", required_argument, 0, 'p'},
        {"help",         no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    args->num_clusters = DEFAULT_NUM_CLUSTERS;
    args->max_load = DEFAULT_MAX_LOAD_SECONDS;
    args->penalization = true;
    args->num_workers = 0;
    
    int opt;
    while ((opt = getopt_long(argc, argv, "i:s:o:k:l:p:h", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'i': args->dataset_path = optarg; break;
            case 's': args->solution_path = optarg; break;
            case 'o': args->results_name = optarg; break;
            case 'k': args->num_clusters = atoi(optarg); break;
            case 'l': args->max_load = atof(optarg); break;
            case 'p': args->penalization = atoi(optarg) != 0; break;
            case 'h': print_usage(argv[0]); exit(0);
            default: return -1;
        }
    }
    
    if (!args->dataset_path || !args->solution_path || !args->results_name) {
        fprintf(stderr, "Error: Missing required arguments\n");
        print_usage(argv[0]);
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    ProgramArgs args = {0};
    if (parse_args(argc, argv, &args) != 0) { MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE); }
    args.num_workers = size;

    ExecutionMetrics metrics = {0};
    double utime_start, stime_start, wtime_start;
    double utime_end, stime_end, wtime_end;

    MPI_Datatype MPI_PWL;
    {
        int blocklens[3] = {1, 1, 1};
        MPI_Aint offsets[3] = {
            offsetof(PointWithLoad, point.lon),
            offsetof(PointWithLoad, point.lat),
            offsetof(PointWithLoad, load)
        };
        MPI_Datatype types[3] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
        MPI_Type_create_struct(3, blocklens, offsets, types, &MPI_PWL);
        MPI_Type_commit(&MPI_PWL);
    }

    PointWithLoad *all_points = NULL;
    size_t total_points = 0;
    int *counts = NULL, *displs = NULL;
    PointWithLoad *local_points = NULL;
    int *local_assignments = NULL;
    int *all_assignments = NULL;
    double *centroid_coords = NULL;
    CentroidSet local_centroids = {0};
    Cluster *clusters = NULL;
    double *cluster_costs = NULL;

    MPI_Barrier(MPI_COMM_WORLD);

    uswtime(&utime_start, &stime_start, &wtime_start);
    double t_total_start = MPI_Wtime();

    double t_part_start = MPI_Wtime();
    if (rank == 0) {
        all_points = load_points_csv(args.dataset_path, &total_points);
        if (!all_points) { fprintf(stderr, "Error dataset\n"); MPI_Abort(MPI_COMM_WORLD, 1); }

        CentroidSet *temp_c = load_centroids_csv(args.solution_path, args.num_clusters);
        if (!temp_c) { fprintf(stderr, "Error centroids\n"); MPI_Abort(MPI_COMM_WORLD, 2); }

        centroid_coords = malloc(args.num_clusters * 2 * sizeof(double));
        for (int i = 0; i < args.num_clusters; ++i) {
            centroid_coords[2*i]   = temp_c->centroids[i].lon;
            centroid_coords[2*i+1] = temp_c->centroids[i].lat;
        }
        free_centroids(temp_c);

        counts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
        size_t base = total_points / size;
        size_t rem = total_points % size;
        displs[0] = 0;
        for (int i = 0; i < size; ++i) {
            counts[i] = (int)(base + (i < (int)rem ? 1 : 0));
            if (i > 0) displs[i] = displs[i-1] + counts[i-1];
        }
    }

    if (rank == 0) metrics.time_partition = MPI_Wtime()-t_part_start;

    double params[3] = {(double)args.num_clusters, args.max_load, (double)args.penalization};
    MPI_Bcast(params, 3, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    int num_clusters = (int)params[0];
    double max_load = params[1];
    bool penalization = (bool)params[2];

    MPI_Bcast(&total_points, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        counts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
    }
    MPI_Bcast(counts, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displs, size, MPI_INT, 0, MPI_COMM_WORLD);

    int local_count = counts[rank];
    local_points = malloc(local_count * sizeof(PointWithLoad));
    if (!local_points) MPI_Abort(MPI_COMM_WORLD, 3);

    if (rank != 0) centroid_coords = malloc(num_clusters * 2 * sizeof(double));
    MPI_Bcast(centroid_coords, num_clusters * 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Scatterv(all_points, counts, displs, MPI_PWL,
                 local_points, local_count, MPI_PWL, 0, MPI_COMM_WORLD);

    local_centroids.centroids = malloc(num_clusters * sizeof(Point));
    local_centroids.num_centroids = num_clusters;
    for (int i = 0; i < num_clusters; ++i) {
        local_centroids.centroids[i].lon = centroid_coords[2*i];
        local_centroids.centroids[i].lat = centroid_coords[2*i+1];
    }
    free(centroid_coords);

    double t_assign_start = MPI_Wtime();
    local_assignments = assign_centroids_to_points(&local_centroids, local_points, local_count);
    if (!local_assignments) MPI_Abort(MPI_COMM_WORLD, 4);
    double t_assign_end = MPI_Wtime();
    metrics.time_assign = t_assign_end - t_assign_start;

    if (rank == 0) {
        all_assignments = malloc(total_points * sizeof(int));
        if (!all_assignments) MPI_Abort(MPI_COMM_WORLD, 5);
    }

    MPI_Gatherv(local_assignments, local_count, MPI_INT,
                all_assignments, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    free(local_points);
    free(local_assignments);
    free(local_centroids.centroids);
    free(counts);
    free(displs);

    double t_clusters_start = MPI_Wtime();
    if (rank == 0) {
        clusters = build_clusters_from_assignments(all_assignments, total_points, 
                                                   all_points, total_points, num_clusters);
        
        free(all_points);
        free(all_assignments);

        metrics.time_cluster = MPI_Wtime() - t_clusters_start;
        double t_cost_start = MPI_Wtime();

        cluster_costs = malloc(num_clusters * sizeof(double));
        for (int i = 0; i < num_clusters; ++i) {
            cluster_costs[i] = calculate_cluster_cost(&clusters[i], DEFAULT_VELOCITY_KMH);
        }

        metrics.time_cost = MPI_Wtime() - t_cost_start;

        double fitness = objective_function(cluster_costs, num_clusters, 
                                            max_load, total_points, penalization);
        
        uswtime(&utime_end, &stime_end, &wtime_end);
        double t_total_end = MPI_Wtime();
        
        metrics.fitness = fitness;
        metrics.time_total = t_total_end - t_total_start;
        metrics.time_assign = t_assign_end - t_assign_start;
        metrics.real_time = wtime_end-wtime_start;
        metrics.user_time = utime_end-utime_start;
        metrics.sys_time = stime_end-stime_start;
        metrics.cpu_wall = 100*((metrics.user_time+metrics.sys_time)/metrics.real_time);
        
        dump_clusters_results(clusters, num_clusters, cluster_costs, args.results_name);
        dump_info_results(&metrics, args.results_name);

        free(cluster_costs);
        free_clusters(clusters, num_clusters);
    }

    MPI_Type_free(&MPI_PWL);
    MPI_Finalize();
    return EXIT_SUCCESS;
}