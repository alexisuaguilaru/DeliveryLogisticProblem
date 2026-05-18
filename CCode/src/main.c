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
    double t_start = MPI_Wtime();

    if (rank == 0) {
        all_points = load_points_csv(args.dataset_path, &total_points);
        if (!all_points) { fprintf(stderr, "[R0] Error dataset\n"); MPI_Abort(MPI_COMM_WORLD, 1); }

        CentroidSet *temp_c = load_centroids_csv(args.solution_path, args.num_clusters);
        if (!temp_c) { fprintf(stderr, "[R0] Error centroides\n"); MPI_Abort(MPI_COMM_WORLD, 2); }

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

    if (rank == 0) {
        clusters = build_clusters_from_assignments(all_assignments, total_points, 
                                                   all_points, total_points, num_clusters);
        
        free(all_points);
        free(all_assignments);
    }

    double t_cost_start = MPI_Wtime();
    ClusterPoint *flat_points = NULL;
    int *c_sizes = NULL, *c_offsets = NULL;
    double *c_loads = NULL;

    int total_pts_int = (int)total_points;
    int num_clust_int = (int)num_clusters;
    int total_bytes   = total_pts_int * (int)sizeof(ClusterPoint);

    #define TAG_SIZES   10
    #define TAG_OFFSETS 11
    #define TAG_LOADS   12
    #define TAG_POINTS  13

    if (rank == 0) {
        flat_points = malloc(total_pts_int * sizeof(ClusterPoint));
        c_sizes     = malloc(num_clust_int * sizeof(int));
        c_offsets   = malloc(num_clust_int * sizeof(int));
        c_loads     = malloc(num_clust_int * sizeof(double));

        if (!flat_points || !c_sizes || !c_offsets || !c_loads) MPI_Abort(MPI_COMM_WORLD, 7);

        size_t cur = 0;
        for (int i = 0; i < num_clust_int; ++i) {
            c_sizes[i]   = (int)clusters[i].count;
            c_loads[i]   = clusters[i].total_load;
            c_offsets[i] = (int)cur;
            if (clusters[i].count > 0) {
                memcpy(flat_points + cur, clusters[i].points, clusters[i].count * sizeof(ClusterPoint));
                cur += clusters[i].count;
            }
        }
        
        for (int dest = 1; dest < size; ++dest) {
            MPI_Send(c_sizes,   num_clust_int, MPI_INT,    dest, TAG_SIZES,   MPI_COMM_WORLD);
            MPI_Send(c_offsets, num_clust_int, MPI_INT,    dest, TAG_OFFSETS, MPI_COMM_WORLD);
            MPI_Send(c_loads,   num_clust_int, MPI_DOUBLE, dest, TAG_LOADS,   MPI_COMM_WORLD);
            MPI_Send(flat_points, total_bytes, MPI_BYTE,   dest, TAG_POINTS,  MPI_COMM_WORLD);
        }
        
    } else {
        c_sizes     = malloc(num_clust_int * sizeof(int));
        c_offsets   = malloc(num_clust_int * sizeof(int));
        c_loads     = malloc(num_clust_int * sizeof(double));
        flat_points = malloc(total_pts_int * sizeof(ClusterPoint));

        if (!c_sizes || !c_offsets || !c_loads || !flat_points) MPI_Abort(MPI_COMM_WORLD, 8);

        MPI_Status status;
        MPI_Recv(c_sizes,   num_clust_int, MPI_INT,    0, TAG_SIZES,   MPI_COMM_WORLD, &status);
        MPI_Recv(c_offsets, num_clust_int, MPI_INT,    0, TAG_OFFSETS, MPI_COMM_WORLD, &status);
        MPI_Recv(c_loads,   num_clust_int, MPI_DOUBLE, 0, TAG_LOADS,   MPI_COMM_WORLD, &status);
        MPI_Recv(flat_points, total_bytes, MPI_BYTE,   0, TAG_POINTS,  MPI_COMM_WORLD, &status);
    }

    double *local_costs = calloc(num_clust_int, sizeof(double));
    for (int i = rank; i < num_clust_int; i += size) {
        Cluster temp_c;
        temp_c.points      = flat_points + c_offsets[i];
        temp_c.count       = c_sizes[i];
        temp_c.total_load  = c_loads[i];
        temp_c.capacity    = temp_c.count;
        local_costs[i]     = calculate_cluster_cost(&temp_c, DEFAULT_VELOCITY_KMH);
    }
    double t_cost_end = MPI_Wtime();
    metrics.time_cost = t_cost_end - t_cost_start;

    free(flat_points);
    free(c_sizes);
    free(c_offsets);
    free(c_loads);

    cluster_costs = malloc(num_clust_int * sizeof(double));
    if (!cluster_costs) MPI_Abort(MPI_COMM_WORLD, 9);

    MPI_Reduce(local_costs, cluster_costs, num_clust_int, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    free(local_costs);

    if (rank == 0) {
        metrics.fitness = objective_function(cluster_costs, num_clusters, 
                                             max_load, total_points, penalization);
        metrics.time_total = MPI_Wtime() - t_start;

        dump_clusters_results(clusters, num_clusters, cluster_costs, args.results_name);
        dump_info_results(&metrics, args.results_name);

        free(cluster_costs);
        for (int i = 0; i < num_clust_int; ++i) {
            if (clusters[i].points) free(clusters[i].points);
        }
        free(clusters);
    }

    MPI_Type_free(&MPI_PWL);
    MPI_Finalize();
    return EXIT_SUCCESS;
}