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
#include "utils.h"

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

    double t_total_start = MPI_Wtime();
    uswtime(&utime_start, &stime_start, &wtime_start);

    size_t total_points = 0;
    if (rank == 0) {
        total_points = count_csv_lines(args.dataset_path);
    }
    if (rank == 0) {
        for (int p = 1; p < size; ++p) {
            MPI_Send(&total_points, 1, MPI_UNSIGNED_LONG_LONG, p, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&total_points, 1, MPI_UNSIGNED_LONG_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    size_t local_start, local_count;
    compute_local_range(total_points, rank, size, &local_start, &local_start);
    compute_local_range(total_points, rank, size, &local_start, &local_count);

    PointWithLoad *local_points = NULL;
    if (local_count > 0) {
        local_points = malloc(local_count * sizeof(PointWithLoad));
        if (!local_points) MPI_Abort(MPI_COMM_WORLD, 1);

        FILE *f = fopen(args.dataset_path, "r");
        if (!f) MPI_Abort(MPI_COMM_WORLD, 2);

        char line[LINE_BUFFER_SIZE];
        size_t idx = 0, current = 0;
        while (fgets(line, sizeof(line), f) && idx < local_start + local_count) {
            if (trim_whitespace(line)[0] == '\0') continue;
            if (idx >= local_start) {
                double values[3];
                if (parse_csv_line_double(line, values, 3) == 3) {
                    local_points[current].point.lon = values[0];
                    local_points[current].point.lat = values[1];
                    local_points[current].load = values[2];
                    current++;
                }
            }
            idx++;
        }
        fclose(f);
    }

    CentroidSet *temp_c = load_centroids_csv(args.solution_path, args.num_clusters);
    if (!temp_c) MPI_Abort(MPI_COMM_WORLD, 3);
    
    double t_assign_start = MPI_Wtime();
    int *local_assignments = assign_centroids_to_points(temp_c, local_points, local_count);
    if (!local_assignments) MPI_Abort(MPI_COMM_WORLD, 4);
    double t_assign_end = MPI_Wtime();
    metrics.time_assign = t_assign_end - t_assign_start;

    free_centroids(temp_c);
    free(local_points); 
    
    if (rank == 0) {
        
        int *recv_counts = calloc(size, sizeof(int));
        recv_counts[0] = (int)local_count;
        for (int p = 1; p < size; ++p) {
            MPI_Recv(&recv_counts[p], 1, MPI_INT, p, p, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        int total_assign = 0;
        for (int p = 0; p < size; ++p) total_assign += recv_counts[p];
        int *all_assignments = malloc(total_assign * sizeof(int));
        memcpy(all_assignments, local_assignments, local_count * sizeof(int));
        
        int offset = (int)local_count;
        for (int p = 1; p < size; ++p) {
            MPI_Recv(all_assignments + offset, recv_counts[p], MPI_INT,
                     p, p + 10000, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            offset += recv_counts[p];
        }

        double t_clusters_start = MPI_Wtime();

        PointWithLoad *all_points = load_points_csv(args.dataset_path, &total_points);
        Cluster *clusters = build_clusters_from_assignments(all_assignments, total_assign,
                                                            all_points, total_points,
                                                            args.num_clusters);
        free(all_points);
        free(all_assignments);
        free(recv_counts);

        metrics.time_cluster = MPI_Wtime() - t_clusters_start;

        double t_cost_start = MPI_Wtime();
        double *cluster_costs = malloc(args.num_clusters * sizeof(double));
        for (int i = 0; i < args.num_clusters; ++i) {
            cluster_costs[i] = calculate_cluster_cost(&clusters[i], DEFAULT_VELOCITY_KMH);
        }
        metrics.time_cost = MPI_Wtime() - t_cost_start;

        double fitness = objective_function(cluster_costs, args.num_clusters,
                                           args.max_load, total_points, args.penalization);

        uswtime(&utime_end, &stime_end, &wtime_end);
        double t_total_end = MPI_Wtime();

        metrics.fitness = fitness;
        metrics.time_total = t_total_end - t_total_start;
        metrics.real_time = wtime_end - wtime_start;
        metrics.user_time = utime_end - utime_start;
        metrics.sys_time = stime_end - stime_start;
        metrics.cpu_wall = 100.0 * ((metrics.user_time + metrics.sys_time) / metrics.real_time);

        dump_clusters_results(clusters, args.num_clusters, cluster_costs, args.results_name);
        dump_info_results(&metrics, args.results_name);

        free(cluster_costs);
        free_clusters(clusters, args.num_clusters);
        free(local_assignments);

    } else {
        int local_cnt = (int)local_count;
        MPI_Send(&local_cnt, 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
        MPI_Send(local_assignments, local_cnt, MPI_INT, 0, rank + 10000, MPI_COMM_WORLD);
        free(local_assignments);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}