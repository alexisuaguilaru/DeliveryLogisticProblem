from time import time

from .ArgparserCLI import MainArgParser
from .LoadData import GetDatasetRangePartitions , GetPointsPartitions
from .CloserCentroid import ReadSolution , AssignCloserCentroidToPoints
from .Cluster import GetClustersFromCentroidsAssignation , WriteClusters
from .Cost import GetClusterCost , ObjectiveFunction
from .Results import DumpClustersResults , DumpInfoResults

from pathlib import Path
from sys import exit

def MainProgram():
    ProgramArgs = MainArgParser.parse_args()
    DATASET_FILE: str = ProgramArgs.dataset
    SOLUTION_FILE: Path = ProgramArgs.solution
    NUM_WORKERS: int = ProgramArgs.workers
    NUM_CLUSTERS: int = ProgramArgs.num_clusters
    MAX_LOAD: float = ProgramArgs.max_load
    PENALIZATION: bool = ProgramArgs.penalization
    NAME_RESULTS: str = ProgramArgs.results

    StartTime_Execution = time()

    BaseDataPath = Path('./data')
    DatasetPath = BaseDataPath/DATASET_FILE

    StartTime_DatasetPartitions = time()
    DatasetRangePartitions: list[int] = GetDatasetRangePartitions(
        DatasetPath,
        NUM_WORKERS,
    )


    PointsPartitions: list[list[tuple[float,float]]] = GetPointsPartitions(
        DatasetPath,
        DatasetRangePartitions,
    )
    EndTime_DatasetPartitions = time()


    StartTime_CentroidsAssignation = time()

    SolutionPath = BaseDataPath/SOLUTION_FILE
    Centroids: list[tuple[float,float]] = ReadSolution(
        SolutionPath,
        NUM_CLUSTERS,
    )

    ClusterAssignation: list[int] = []
    for points_partition in PointsPartitions:
        centroids_assignation: list[int] = AssignCloserCentroidToPoints(
            Centroids,
            points_partition,
        )
        ClusterAssignation += centroids_assignation
    EndTime_CentroidsAssignation = time()

    exit(0)

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