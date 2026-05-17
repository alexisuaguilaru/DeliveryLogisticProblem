from statistics import stdev
from math import sqrt

from .Distance import CalculateTransferTime

def GetClusterCost(
        ClusterPoints: list[tuple[float,float]],
        ClusterLoad: float,
        Velocity: float = 50,
    ) -> float:

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
        
        Cost += ClusterLoad

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