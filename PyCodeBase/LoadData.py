from csv import reader

from pathlib import Path

def GetDatasetRangePartitions(
        DatasetPath: Path,
        NumNodes: int,
    ) -> list[int]:

    NumRows = _CountRows(DatasetPath)
    RowsByNode = NumRows//NumNodes
    ResidualRows = NumRows%NumNodes

    Partitions = [RowsByNode for _ in range(NumNodes)]
    for index_partition in range(ResidualRows):
        Partitions[index_partition] += 1

    RangePartitions = [0]
    for partition in Partitions:
        range_partition = RangePartitions[-1]+partition
        RangePartitions.append(range_partition)

    return RangePartitions

def GetPointsPartitions(
        DatasetPath: Path,
        RangePartitions: list[int],
    ) -> list[list[tuple[float,float]]]:

    PointsPartitions = []

    with open(DatasetPath) as DatasetFile:
        RowPoints = reader(DatasetFile)

        for start_partition , end_partition in zip(RangePartitions,RangePartitions[1:]):
            point_partition = []
            range_partition = range(start_partition,end_partition)

            for _ , row_point in zip(range_partition,RowPoints):
                point_partition.append(_ConvertRowToPoint(row_point))
            
            PointsPartitions.append(point_partition)

    return PointsPartitions

def _CountRows(
        DatasetPath: Path,
    ) -> int:

    with open(DatasetPath) as dataset_file:
        return sum(1 for _ in dataset_file)
    
def _ConvertRowToPoint(
        RowPoint: list[str],
    ):

    return (
        float(RowPoint[0]),
        float(RowPoint[1]),
    )