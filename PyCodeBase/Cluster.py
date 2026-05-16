from pathlib import Path

def GetClustersFromCentroidsAssignation(
        ClusterPartition: list[int],
        NumClusters: int,
    ) -> list[list[int]]:

    Clusters = [[] for _ in range(NumClusters)]
    for index_point , index_cluster in enumerate(ClusterPartition):
        Clusters[index_cluster].append(index_point)

    return Clusters

def WriteClusters(
        DatasetPath: Path,
        Clusters: list[list[int]],
    ) -> list[Path]:

    ListClustersPath = []

    for cluster_id , cluster in enumerate(Clusters,1):
        cluster_name = DatasetPath.parent/f'cluster_{cluster_id}.csv'
        ListClustersPath.append(cluster_name)

        with open(DatasetPath) as DatasetFile , open(ListClustersPath[-1],'w') as cluster_file:
            HeaderLine = DatasetFile.readline()
            cluster_file.write(HeaderLine)

            index_cluster = 0
            for index_row , data_point_row in enumerate(DatasetFile):
                if index_row == cluster[index_cluster]:
                    cluster_file.write(data_point_row)
                    index_cluster += 1
                
                if index_cluster == len(cluster):
                    break

    return ListClustersPath