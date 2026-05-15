from .ArgparserCLI import MainArgParser
from .LoadData import GetDatasetPartitions , WriteDatasetPartitions
from .CloserCentroid import AssignCloserCentroidToPoints
from .Cluster import GetClustersFromCentroidsAssignation , WriteClusters
from .Cost import GetClusterCost , ObjectiveFunction

def MainProgram():
    ProgramArgs = MainArgParser.parse_args()
    DATASET_PATH = ProgramArgs.input_dataset
    SOLUTION_PATH = ProgramArgs.solution
    NUM_WORKERS= ProgramArgs.workers
    SOLUTION_PATH = ProgramArgs.solution
    IDENTIFIER_COLUMN = ProgramArgs.identifier
    LONGITUDE_COLUMN = ProgramArgs.longitude
    LATITUDE_COLUMN = ProgramArgs.latitude
    LOAD_COLUMN = ProgramArgs.load
    NUM_CLUSTERS = ProgramArgs.num_clusters
    MAX_LOAD = ProgramArgs.max_load
    PENALIZATION = ProgramArgs.penalization

    DatasetPartitions = GetDatasetPartitions(
        DATASET_PATH,
        NUM_WORKERS,
    )

    ListPartitionsPath = WriteDatasetPartitions(
        DATASET_PATH,
        DatasetPartitions,
    )

    ClusterPartition = []
    for partition_path in ListPartitionsPath:
        centroids_assignation = AssignCloserCentroidToPoints(
            partition_path,
            SOLUTION_PATH,
            LONGITUDE_COLUMN,
            LATITUDE_COLUMN,
            NUM_CLUSTERS,
        )
        ClusterPartition += centroids_assignation

    Clusters = GetClustersFromCentroidsAssignation(
        ClusterPartition,
        NUM_CLUSTERS,
    )

    ListClustersPath = WriteClusters(
        DATASET_PATH,
        Clusters,
    )

    ClustersCosts = []
    for cluster_path in ListClustersPath:
        cluster_cost = GetClusterCost(
            cluster_path,
            LONGITUDE_COLUMN,
            LATITUDE_COLUMN,
            LOAD_COLUMN
        )

        ClustersCosts.append(cluster_cost)

    NumPoints = sum(map(len,Clusters))
    Fitness = ObjectiveFunction(
        ClustersCosts,
        MAX_LOAD,
        NumPoints,
        PENALIZATION,
    )

    print(Fitness)