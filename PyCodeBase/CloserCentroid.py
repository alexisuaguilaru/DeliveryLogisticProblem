from pathlib import Path
from csv import reader

from .Distance import CalculateTransferTime
from .Utils import ReadDatasetPartition

def AssignCloserCentroidToPoints(
        PartitionPath: Path,
        SolutionPath: Path,
        LongitudeColumn: str,
        LatitudeColumn: str,
        NumClusters: int,
        Velocity: float = 50,
    ) -> list[int]:

    Centroids = _ReadSolution(
        SolutionPath,
        NumClusters,
    )

    PartitionPoints = ReadDatasetPartition(
        PartitionPath,
        LongitudeColumn,
        LatitudeColumn,
    )

    CloserCentroidToPoints = []
    for point in PartitionPoints:
        closer_centroid = None
        min_distance = float('inf')

        for index , centroid in enumerate(Centroids):
            distance = CalculateTransferTime(point,centroid,Velocity)
            if distance < min_distance:
                min_distance = distance
                closer_centroid = index

        CloserCentroidToPoints.append(closer_centroid)

    return CloserCentroidToPoints

def _ReadSolution(
        SolutionPath: Path,
        NumClusters: int,
    ) -> list[tuple[float,float]]:

    Centroids = []
    with open(SolutionPath) as SolutionFile:
        Solution = list(reader(SolutionFile))[0]

        for index_centroid in range(NumClusters):
            centroid_x = float(Solution[2*index_centroid])
            centroid_y = float(Solution[2*index_centroid+1])
            centroid = (centroid_x,centroid_y)
            Centroids.append(centroid)

    return Centroids