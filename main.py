import subprocess
from itertools import product
import json

from typing import Union
from pathlib import Path

def RunExperiment():
    try:
        subprocess.run(
            './main.sh',
            shell = True,
        )
        return True
    except:
        return False  

def GridParameters(
        ArgumentsList: dict[str,list],
    ):

    Argument_Names = list(ArgumentsList.keys())
    Arguments_Values = product(*ArgumentsList.values())

    for argument_values in Arguments_Values:
        yield zip(Argument_Names,argument_values)

def WriteParametersToConfigFile(
        ExecutionID: int,
        Parameters: dict[str,Union[str,int,float]],
        ConfigFile: Union[str,Path] = 'config.env',
    ):

    with open(ConfigFile,'w') as config_file:
        config_file.write(f'ID={ExecutionID}\n')
        for name , value in Parameters.items():
            if isinstance(value,str):
                config_file.write(f'{name}="{value}"\n')
            elif isinstance(value,float):
                config_file.write(f'{name}={value:.8f}\n')
            else:
                config_file.write(f'{name}={value}\n')


if __name__ == '__main__':
    with open('Experiments.json') as json_experiments:
        ArgumentsList = json.load(json_experiments)
        NumberExperiments = ArgumentsList['EXPERIMENTS']
        del ArgumentsList['EXPERIMENTS']
    
    for experiment_id , parameters in enumerate(GridParameters(ArgumentsList),1):
        print('='*15)
        print(f'## START: EXPERIMENTS {experiment_id:03} ##\n')
        
        parameters = dict(parameters)
        NumNodes = parameters['NODES']
        for run_id in range(1,NumberExperiments+1):
            parameters['OUTPUT_FILE'] = f'Nodes{NumNodes:03}/Run_{experiment_id:04}'
            WriteParametersToConfigFile(run_id,parameters)
            print(f'### START: RUN {run_id:03} ###')
            if RunExperiment():
                print(f'### END: RUN {run_id:03} ###')
            else:
                print(f'### FAIL: RUN {run_id:03} ###')
        
        print(f'\n## END: EXPERIMENTS {experiment_id:03} ##')
        print('='*15)
        print()