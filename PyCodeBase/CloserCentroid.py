from pathlib import Path

from .Distance import _HaversineDistance
from .Utils import ReadDatasetPartition

def ReadSolution(
        SolutionPath: Path,
        NumClusters: int,
    ) -> list[tuple[float,float]]:

    Centroids = []
    with open(SolutionPath) as SolutionFile:
        Solution = SolutionFile.read().strip().split(',')

        for index_centroid in range(NumClusters):
            centroid_x = float(Solution[2*index_centroid])
            centroid_y = float(Solution[2*index_centroid+1])
            centroid = (centroid_x,centroid_y)
            Centroids.append(centroid)

    return Centroids

def AssignCloserCentroidToPoints(
        Centroids: list[tuple[float,float]],
        PointsPartition: list[tuple[float,float]],
    ) -> list[int]:

    CloserCentroidToPoints = []
    for point in PointsPartition:
        closer_centroid = None
        min_distance = float('inf')

        for index , centroid in enumerate(Centroids):
            distance = _HaversineDistance(point,centroid)
            if distance < min_distance:
                min_distance = distance
                closer_centroid = index

        CloserCentroidToPoints.append(closer_centroid)
    
    return CloserCentroidToPoints