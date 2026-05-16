from pathlib import Path
from csv import reader
from statistics import stdev
from math import sqrt

from .Utils import ReadDatasetPartition
from .Distance import CalculateTransferTime

def GetClusterCost(
        ClusterPath: Path,
        LongitudeColumn: str,
        LatitudeColumn: str,
        LoadColumn: str,
        Velocity: float = 50,
    ) -> float:

    ClusterPoints = ReadDatasetPartition(
        ClusterPath,
        LongitudeColumn,
        LatitudeColumn,
    )
    ClusterPoints.sort(key = lambda point: point[0])

    NumPoints = len(ClusterPoints)
    Cost = 0
    
    if 1 < NumPoints:
        for index_i , index_j in zip(range(NumPoints),range(1,NumPoints)):
            point_i = ClusterPoints[index_i]
            point_j = ClusterPoints[index_j]
            Cost += CalculateTransferTime(point_i,point_j,Velocity)

        point_i = ClusterPoints[-1]
        point_j = ClusterPoints[0]
        Cost += CalculateTransferTime(point_i,point_j,Velocity)
        
        Cost += _GetTotalLoad(
            ClusterPath,
            LoadColumn,
        )

    return Cost

def ObjectiveFunction(
        ClustersCosts: list[float],
        MaxLoad: float,
        NumPoints: int,
        Penalization: bool,
    ) -> float:

    Fitness = stdev(ClustersCosts)

    if Penalization:
        LoadConstraint = sum(MaxLoad<cluster_cost for cluster_cost in ClustersCosts)
        Fitness += LoadConstraint*sqrt(NumPoints)

    return Fitness

def _GetTotalLoad(
        ClusterPath: Path,
        LoadColumn: str,
    ) -> float:

    with open(ClusterPath) as ClusterFile:
        IndexLoad = _GetIndexLoadFeature(
            ClusterFile.readline().strip().split(','),
            LoadColumn,
        )

        TotalLoad = 0
        for data_point_row in reader(ClusterFile):
            TotalLoad += float(data_point_row[IndexLoad])
    
    return TotalLoad

def _GetIndexLoadFeature(
        DatasetHeader: list[str],
        LoadColumn: str,
    ) -> int:

    IndexLoad = None

    for index_label , column_label in enumerate(DatasetHeader):
        if column_label == LoadColumn:
            IndexLoad = index_label
            break
    
    return IndexLoad