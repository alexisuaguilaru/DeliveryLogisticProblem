from time import time

from .ArgparserCLI import MainArgParser
from .LoadData import GetDatasetPartitions , WriteDatasetPartitions
from .CloserCentroid import AssignCloserCentroidToPoints
from .Cluster import GetClustersFromCentroidsAssignation , WriteClusters
from .Cost import GetClusterCost , ObjectiveFunction
from .Results import DumpClustersResults , DumpInfoResults

from pathlib import Path

def MainProgram():
    ProgramArgs = MainArgParser.parse_args()
    DATASET_PATH: Path = ProgramArgs.input_dataset
    SOLUTION_PATH: Path = ProgramArgs.solution
    NUM_WORKERS: int = ProgramArgs.workers
    LONGITUDE_COLUMN: str = ProgramArgs.longitude
    LATITUDE_COLUMN: str = ProgramArgs.latitude
    LOAD_COLUMN: str = ProgramArgs.load
    NUM_CLUSTERS: int = ProgramArgs.num_clusters
    MAX_LOAD: float = ProgramArgs.max_load
    PENALIZATION: bool = ProgramArgs.penalization
    OUTPUT_PATH: Path = ProgramArgs.output_path
    NAME_RESULTS: str = ProgramArgs.name_results

    StartTime_Execution = time()

    StartTime_DatasetPartitions = time()
    DatasetPartitions: list[int] = GetDatasetPartitions(
        DATASET_PATH,
        NUM_WORKERS,
    )

    ListPartitionsPath: list[Path] = WriteDatasetPartitions(
        DATASET_PATH,
        DatasetPartitions,
    )
    EndTime_DatasetPartitions = time()

    StartTime_CentroidsAssignation = time()
    ClusterPartition: list[int] = []
    for partition_path in ListPartitionsPath:
        centroids_assignation: list[int] = AssignCloserCentroidToPoints(
            partition_path,
            SOLUTION_PATH,
            LONGITUDE_COLUMN,
            LATITUDE_COLUMN,
            NUM_CLUSTERS,
        )
        ClusterPartition += centroids_assignation
    EndTime_CentroidsAssignation = time()

    StartTime_Clusters = time()
    Clusters: list[list[int]] = GetClustersFromCentroidsAssignation(
        ClusterPartition,
        NUM_CLUSTERS,
    )

    ListClustersPath: list[Path] = WriteClusters(
        DATASET_PATH,
        Clusters,
    )
    EndTime_Clusters = time()

    StartTime_CalculateCosts = time()
    ClustersCosts: list[float] = []
    for cluster_path in ListClustersPath:
        cluster_cost: float = GetClusterCost(
            cluster_path,
            LONGITUDE_COLUMN,
            LATITUDE_COLUMN,
            LOAD_COLUMN
        )

        ClustersCosts.append(cluster_cost)

    NumPoints: int = sum(map(len,Clusters))
    Fitness: float = ObjectiveFunction(
        ClustersCosts,
        MAX_LOAD,
        NumPoints,
        PENALIZATION,
    )
    EndTime_CalculateCosts = time()

    EndTime_Execution = time()

    DumpClustersResults(
        Clusters,
        ClustersCosts,
        OUTPUT_PATH,
        NAME_RESULTS,
    )

    DumpInfoResults(
        Fitness,
        EndTime_Execution-StartTime_Execution,
        EndTime_DatasetPartitions-StartTime_DatasetPartitions,
        EndTime_CentroidsAssignation-StartTime_CentroidsAssignation,
        EndTime_Clusters-StartTime_Clusters,
        EndTime_CalculateCosts-StartTime_CalculateCosts,
        OUTPUT_PATH,
        NAME_RESULTS,
    )