import argparse
from pathlib import Path

MainArgParser = argparse.ArgumentParser(
    prog = 'LogisticDelivery',
    description = 'A simple entrypoint for the objective function in Logistic Delivery write with Python.'
)

MainArgParser.add_argument(
    '-in', '--dataset',
    required = True,
    help = 'Name of the CSV file with the coordinates (longitude,latitude) and loads of every point.',
    type = str,
)

MainArgParser.add_argument(
    '-s', '--solution',
    required = True,
    help = 'Name of the CSV file with the values of a solution.',
    type = str,
)

MainArgParser.add_argument(
    '-k', '--num-clusters',
    default = 10,
    help = 'Number of clusters of the solution.',
    type = int,
)

MainArgParser.add_argument(
    '-ml','--max-load',
    default = 12*3600,
    required = True,
    help = 'Maximum time to complete a route per cluster.',
    type = int,
)

MainArgParser.add_argument(
    '-p','--penalization',
    default = 1,
    choices = [0,1],
    help = 'Include penalization in the objective function.',
    type = int,
)

MainArgParser.add_argument(
    '-out', '--results',
    required = True,
    help = 'Results name or identifier.',
    type = str,
)

MainArgParser.add_argument(
    '-nj', '--workers',
    required = True,
    help = 'Number of compute nodes.',
    type = int,
)