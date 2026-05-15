from pathlib import Path
from csv import reader

from .Distance import CalculateTransferTime

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

    PartitionPoints = _ReadDatasetPartition(
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

def _ReadDatasetPartition(
        PartitionPath: Path,
        LongitudeColumn: str,
        LatitudeColumn: str,
    ):

    with open(PartitionPath) as PartitionFile:
        IndexLongitude , IndexLatitude = _GetIndexSpatialFeatures(
            PartitionFile.readline().split(','),
            LongitudeColumn,
            LatitudeColumn,
        )

        Points = []
        for data_point_row in reader(PartitionFile):
            point = (
                float(data_point_row[IndexLongitude]),
                float(data_point_row[IndexLatitude]),
            )
            Points.append(point)

    return Points

def _GetIndexSpatialFeatures(
        DatasetHeader: list[str],
        LongitudeColumn: str,
        LatitudeColumn: str,
    ) -> tuple[int,int]:

    IndexLongitude = None
    IndexLatitude = None

    for index_label , column_label in enumerate(DatasetHeader):
        if column_label == LongitudeColumn:
            IndexLongitude = index_label
        elif column_label == LatitudeColumn:
            IndexLatitude = index_label
    
    return IndexLongitude , IndexLatitude