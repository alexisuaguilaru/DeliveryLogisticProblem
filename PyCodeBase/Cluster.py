from csv import reader

from .Utils import ConvertRowToPoint

from pathlib import Path
from typing import Generator

def GetClustersFromCentroidsAssignation(
        ClusterAssignation: list[int],
        NumClusters: int,
        DatasetPath: Path,
    ) -> tuple[list[list[tuple[float,float]]],list[float],list[list[int]]]:

    Clusters = [[] for _ in range(NumClusters)]
    ClustersLoads = [0 for _ in range(NumClusters)]
    ClustersIndexes = [[] for _ in range(NumClusters)]
    Points = _GetPoints(DatasetPath)

    for index_point , ((point,load) , index_cluster) in enumerate(zip(Points,ClusterAssignation)):
        Clusters[index_cluster].append(point)
        ClustersLoads[index_cluster] += load
        ClustersIndexes[index_cluster].append(index_point)

    return Clusters , ClustersLoads , ClustersIndexes

def _GetPoints(
        DatasetPath: Path,
    ) -> Generator[tuple[tuple[float,float],float]]:

    with open(DatasetPath) as DatasetFile:
        RowPoints = reader(DatasetFile)

        for row_point in RowPoints:
            yield ConvertRowToPoint(row_point) , float(row_point[2])