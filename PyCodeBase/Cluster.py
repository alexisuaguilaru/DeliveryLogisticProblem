from csv import reader

from .Utils import ConvertRowToPoint

from pathlib import Path
from typing import Generator

def GetClustersFromCentroidsAssignation(
        ClusterAssignation: list[int],
        NumClusters: int,
        DatasetPath: Path,
    ) -> tuple[list[list[tuple[float,float]]],list[float]]:

    Clusters = [[] for _ in range(NumClusters)]
    ClustersLoads = [0 for _ in range(NumClusters)]
    Points = _GetPoints(DatasetPath)

    for (point,load) , index_cluster in zip(Points,ClusterAssignation):
        Clusters[index_cluster].append(point)
        ClustersLoads[index_cluster] += load

    return Clusters , ClustersLoads

def _GetPoints(
        DatasetPath: Path,
    ) -> Generator[tuple[tuple[float,float],float]]:

    with open(DatasetPath) as DatasetFile:
        RowPoints = reader(DatasetFile)

        for row_point in RowPoints:
            yield ConvertRowToPoint(row_point) , float(row_point[2])