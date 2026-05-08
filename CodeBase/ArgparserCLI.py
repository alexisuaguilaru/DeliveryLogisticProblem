import argparse
from pathlib import Path

MainArgParser = argparse.ArgumentParser(
    prog = 'Logistic Delivery Optimizer',
)

MainArgParser.add_argument(
    '-if', '--input-file',
    required = True,
    help = 'Path to the CSV file with the coordinates and loads of every point.',
    type = Path,
)

MainArgParser.add_argument(
    '-lon','--longitude',
    default = 'lon',
    help = 'Dataset feature related to point longitude',
    type = str,
)

MainArgParser.add_argument(
    '-lat','--latitude',
    default = 'lat',
    help = 'Dataset feature related to point latitude',
    type = str,
)

MainArgParser.add_argument(
    '-l','--load',
    default = 'load',
    help = 'Dataset feature related to point load',
    type = str,
)

MainArgParser.add_argument(
    '-op','--output-path',
    default = 'output_files',
    required = True,
    help = 'Path of the folder for output or results.',
    type = Path,
)

MainArgParser.add_argument(
    '-nd','--name-dataset',
    required = True,
    help = 'Dataset name or identifier.',
    type = str,
)

MainArgParser.add_argument(
    '-ml','--max-load',
    default = 12*3600,
    required = True,
    help = 'Maximum time to complete a route per cluster.',
    type = int,
)

MainArgParser.add_argument(
    '-s','--seed',
    default = 1234,
    help = 'Execution/Program random seed.',
    type = int,
)