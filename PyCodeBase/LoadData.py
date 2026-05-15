from pathlib import Path

def GetDatasetPartitions(
        DatasetPath: Path,
        NumNodes: int,
    ) -> list[int]:

    NumRows = _CountRows(DatasetPath)-1
    RowsByNode = NumRows//NumNodes
    ResidualRows = NumRows%NumNodes

    Partitions = [RowsByNode for _ in range(NumNodes)]
    for index_partition in range(ResidualRows):
        Partitions[index_partition] += 1

    return Partitions

def WriteDatasetPartitions(
        DatasetPath: Path,
        PartitionsSize: list[int]
    ) -> list[Path]:

    ListPathPartitions = []

    with open(DatasetPath) as DatasetFile:
        HeaderLine = DatasetFile.readline()

        for partition_id , num_points in enumerate(PartitionsSize,1):
            partition_name = DatasetPath.stem+f'_{partition_id}'+DatasetPath.suffix
            ListPathPartitions.append(DatasetPath.parent/partition_name)

            with open(ListPathPartitions[-1],'w') as dataset_partition:
                dataset_partition.write(HeaderLine)
                
                for _ , row_data in zip(range(num_points),DatasetFile):
                    dataset_partition.write(row_data)

    return ListPathPartitions

def _CountRows(
        DatasetPath: Path,
    ) -> int:

    with open(DatasetPath) as dataset_file:
        return sum(1 for _ in dataset_file)