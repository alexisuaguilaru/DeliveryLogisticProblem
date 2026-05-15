from pathlib import Path
from csv import reader

def ReadDatasetPartition(
        PartitionPath: Path,
        LongitudeColumn: str,
        LatitudeColumn: str,
    ) -> list[tuple[float,float]]:

    with open(PartitionPath) as PartitionFile:
        IndexLongitude , IndexLatitude = _GetIndexSpatialFeatures(
            PartitionFile.readline().strip().split(','),
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