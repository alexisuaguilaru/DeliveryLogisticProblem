import argparse
from pathlib import Path

MainArgParser = argparse.ArgumentParser(
    prog = 'LogisticDelivery',
    description = 'A simple entrypoint for the objective function in Logistic Delivery write with Python.'
)

MainArgParser.add_argument(
    '-if', '--input-dataset',
    required = True,
    help = 'Path to the CSV file with the coordinates, loads and identifier of every point.',
    type = Path,
)

MainArgParser.add_argument(
    '-id', '--identifier',
    default = 'id',
    help = 'Dataset feature related to point identifier.',
    type = str,
)

MainArgParser.add_argument(
    '--longitude',
    default = 'lon',
    help = 'Dataset feature related to point longitude',
    type = str,
)

MainArgParser.add_argument(
    '--latitude',
    default = 'lat',
    help = 'Dataset feature related to point latitude',
    type = str,
)

MainArgParser.add_argument(
    '-l', '--load',
    default = 'load',
    help = 'Dataset feature related to point load.',
    type = str,
)

MainArgParser.add_argument(
    '-s', '--solution',
    required = True,
    help = 'Path to the CSV file with the values of a solution.',
    type = str,
)

MainArgParser.add_argument(
    '-k', '--num-clusters',
    default = 10,
    help = 'Number of clusters to form.',
    type = int,
)

MainArgParser.add_argument(
    '-d','-distance',
    default = 'haversine',
    help = 'Distance function name to calculate distances between centroids and points',
    type = str, 
)

MainArgParser.add_argument(
    '-oc','--order-column',
    required = True,
    help = 'Column on which the dataset is ordered.',
    type = str,   
)

MainArgParser.add_argument(
    '-op', '--output-path',
    default = 'output_files',
    required = True,
    help = 'Path of the folder for output or results.',
    type = Path,
)

MainArgParser.add_argument(
    '-nd', '--name-dataset',
    required = True,
    help = 'Dataset name or identifier.',
    type = str,
)

MainArgParser.add_argument(
    '-nj', '--workers',
    required = True,
    help = 'Number of compute nodes.',
    type = int,
)