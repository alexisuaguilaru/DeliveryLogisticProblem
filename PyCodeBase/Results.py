from pathlib import Path

def DumpClustersResults(
        Clusters: list[list[int]],
        ClustersCosts: list[float],
        OutputPath: Path,
        NameResults: str,
    ) -> None:

    OutputPath.mkdir(parents=True,exist_ok=True)

    with open(OutputPath/f'{NameResults}_clusters.csv','w') as clusters_file:
        clusters_file.write('Cost,Cluster\n')

        for cost , cluster in zip(ClustersCosts,Clusters):
            clusters_file.write(f'{cost},')
            str_cluster = ','.join(map(str,cluster))
            clusters_file.write(f'"[{str_cluster}]"\n')

def DumpInfoResults(
        Fitness: float,
        TimeExecution: float,
        TimeDatasetPartition: float,
        TimeCentroidsAssignation: float,
        TimeClusters: float,
        TimeCalculateCosts: float,
        OutputPath: Path,
        NameResults: str,
    ) -> None:

    OutputPath.mkdir(parents=True,exist_ok=True)

    with open(OutputPath/f'{NameResults}_info.csv','w') as clusters_file:
        clusters_file.write('Parameter,Value\n')
        clusters_file.write(f'Fitness,{Fitness}\n')
        clusters_file.write(f'TimeExecution,{TimeExecution}\n')
        clusters_file.write(f'TimeDatasetPartition,{TimeDatasetPartition}\n')
        clusters_file.write(f'TimeCentroidsAssignation,{TimeCentroidsAssignation}\n')
        clusters_file.write(f'TimeClusters,{TimeClusters}\n')
        clusters_file.write(f'TimeCalculateCosts,{TimeCalculateCosts}\n')