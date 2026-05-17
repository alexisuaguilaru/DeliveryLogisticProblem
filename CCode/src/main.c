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
    fprintf(stderr, "Help: %s -i <dataset> -s <solution> -o <results> -n <nodes> [options]\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -k INT    Number of clusters to form (default: %d)\n", DEFAULT_NUM_CLUSTERS);
    fprintf(stderr, "  -l INT    Max time to complete a route per cluster (default: %d)\n", DEFAULT_MAX_LOAD_SECONDS);
    fprintf(stderr, "  -p {0,1}  Active penalization in the objective function (default: 1)\n");
    fprintf(stderr, "  -h        Show help \n");
}

static int parse_args(int argc, char **argv, ProgramArgs *args) {
    static struct option long_opts[] = {
        {"dataset",      required_argument, 0, 'i'},
        {"solution",     required_argument, 0, 's'},
        {"results",      required_argument, 0, 'o'},
        {"nodes",        required_argument, 0, 'n'},
        {"num-clusters", required_argument, 0, 'k'},
        {"max-load",     required_argument, 0, 'l'},
        {"penalization", required_argument, 0, 'p'},
        {"help",         no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    args->num_clusters = DEFAULT_NUM_CLUSTERS;
    args->max_load = DEFAULT_MAX_LOAD_SECONDS;
    args->penalization = true;
    args->num_workers = 1;
    
    int opt;
    while ((opt = getopt_long(argc, argv, "i:s:o:n:k:l:p:h", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'i': args->dataset_path = optarg; break;
            case 's': args->solution_path = optarg; break;
            case 'o': args->results_name = optarg; break;
            case 'n': args->num_workers = atoi(optarg); break;
            case 'k': args->num_clusters = atoi(optarg); break;
            case 'l': args->max_load = atof(optarg); break;
            case 'p': args->penalization = atoi(optarg) != 0; break;
            case 'h': print_usage(argv[0]); exit(0);
            default: return -1;
        }
    }
    
    if (!args->dataset_path || !args->solution_path || !args->results_name || args->num_workers <= 0) {
        fprintf(stderr, "Error: Missing required arguments\n");
        print_usage(argv[0]);
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    ProgramArgs args = {0};
    if (parse_args(argc, argv, &args) != 0) return EXIT_FAILURE;
    
    ExecutionMetrics metrics = {0};
    double start_total = clock();
    
    double start_partition = clock();
    size_t *partition_sizes = calloc(args.num_workers, sizeof(size_t));
    int *partitions = get_dataset_range_partitions(args.dataset_path, args.num_workers, partition_sizes);
    if (!partitions) {
        fprintf(stderr, "Error: Invalid dataset partition\n");
        return EXIT_FAILURE;
    }
    metrics.time_partition = (clock() - start_partition) / (double)CLOCKS_PER_SEC;
    
    size_t total_points;
    PointWithLoad *points = load_points_csv(args.dataset_path, &total_points);
    if (!points) {
        fprintf(stderr, "Error: No dataset to load\n");
        free(partitions); free(partition_sizes);
        return EXIT_FAILURE;
    }
    
    double start_assign = clock();
    CentroidSet *centroids = load_centroids_csv(args.solution_path, args.num_clusters);
    if (!centroids) {
        fprintf(stderr, "Error: No centroids to load\n");
        free_points(points, total_points); free(partitions); free(partition_sizes);
        return EXIT_FAILURE;
    }
    
    int *assignments = assign_centroids_to_points(centroids, points, total_points);
    if (!assignments) {
        fprintf(stderr, "Error: Fail to assign centroids\n");
        free_centroids(centroids); free_points(points, total_points);
        free(partitions); free(partition_sizes);
        return EXIT_FAILURE;
    }
    metrics.time_assign = (clock() - start_assign) / (double)CLOCKS_PER_SEC;
    
    double start_cluster = clock();
    Cluster *clusters = build_clusters_from_assignments(
        assignments, total_points, points, total_points, args.num_clusters
    );
    if (!clusters) {
        fprintf(stderr, "Error: Fail to form clusters\n");
        free(assignments); free_centroids(centroids); free_points(points, total_points);
        free(partitions); free(partition_sizes);
        return EXIT_FAILURE;
    }
    metrics.time_cluster = (clock() - start_cluster) / (double)CLOCKS_PER_SEC;
    
    double start_cost = clock();
    double *cluster_costs = malloc(args.num_clusters * sizeof(double));
    if (!cluster_costs) {
        fprintf(stderr, "Error: Fail to assign memory for clusters cost\n");
        free_clusters(clusters, args.num_clusters); free(assignments);
        free_centroids(centroids); free_points(points, total_points);
        free(partitions); free(partition_sizes);
        return EXIT_FAILURE;
    }
    
    for (int i = 0; i < args.num_clusters; ++i) {
        cluster_costs[i] = calculate_cluster_cost(&clusters[i], DEFAULT_VELOCITY_KMH);
    }
    
    metrics.fitness = objective_function(
        cluster_costs, args.num_clusters, args.max_load, total_points, args.penalization
    );
    metrics.time_cost = (clock() - start_cost) / (double)CLOCKS_PER_SEC;
    
    metrics.time_total = (clock() - start_total) / (double)CLOCKS_PER_SEC;
    
    if (dump_info_results(&metrics, args.results_name) != 0) {
        fprintf(stderr, "Warning: Error to dump results\n");
    }
    
    printf("Fitness: %f\n", metrics.fitness);
    printf("Total time: %.3f s\n", metrics.time_total);
    
    free(cluster_costs);
    free_clusters(clusters, args.num_clusters);
    free(assignments);
    free_centroids(centroids);
    free_points(points, total_points);
    free(partitions);
    free(partition_sizes);
    
    return EXIT_SUCCESS;
}