from .ArgparserCLI import MainArgParser
from .LoadData import GetDatasetPartitions , WriteDatasetPartitions

def MainProgram():
    ProgramArgs = MainArgParser.parse_args()
    DATASET_PATH = ProgramArgs.input_dataset

    DatasetPartitions = GetDatasetPartitions(
        DATASET_PATH,
        ProgramArgs.workers
    )

    ListPathPartitions = WriteDatasetPartitions(
        DATASET_PATH,
        DatasetPartitions
    )