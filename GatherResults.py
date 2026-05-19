from pathlib import Path
from csv import reader

if __name__ == '__main__':
    InfoFiles = list(Path('./results').glob('Node*/*info*'))

    with open('ResultsExperiments.csv','w') as results_file:
        results_file.write('Nodes,Run,RealTime,UserTime,SysTime,CPUWall\n')
        for info_file in InfoFiles:
            _ , nodes , run = info_file.parts
            num_nodes = int(nodes[-3:]) 
            run_id = int(run[4:8])

            with open(info_file) as run_csv:
                run_results = reader(run_csv)

                for result in run_results:
                    if result[0] == 'RealTime':
                        real_time = result[1]
                    elif result[0] == 'UserTime':
                        user_time = result[1]
                    elif result[0] == 'SysTime':
                        sys_time = result[1]
                    elif result[0] == 'CPUWall':
                        cpu_wall = result[1]

            results_file.write(f'{num_nodes},{run_id},{real_time},{user_time},{sys_time},{cpu_wall}\n')