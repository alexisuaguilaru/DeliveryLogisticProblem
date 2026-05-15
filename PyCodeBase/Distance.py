from math import radians , cos , acos

EARTH_RADIUS = 6378.388

def CalculateTransferTime(
        StartPoint: tuple[float,float],
        EndPoint: tuple[float,float],
        Velocity: float,
    ) -> float:

    Distance = _HaversineDistance(StartPoint,EndPoint)
    return 3600*Distance/Velocity

def _HaversineDistance(
        StartPoint: tuple[float,float],
        EndPoint: tuple[float,float],
    ) -> float:

    Point_1_x = radians(StartPoint[0])
    Point_1_y = radians(StartPoint[1])
    Point_2_x = radians(EndPoint[0])
    Point_2_y = radians(EndPoint[1])

    CosDif_x = cos(Point_1_x-Point_2_x)
    CosDif_y = cos(Point_1_y-Point_2_y)
    CosSum_x = cos(Point_1_x+Point_2_x)
    CosSum_y = cos(Point_1_y+Point_2_y)

    Term_1 = (1+CosDif_x)*CosDif_y
    Term_2 = (1-CosDif_x)*CosSum_y

    Term_acos = acos(0.5*(Term_1-Term_2))
    return EARTH_RADIUS*Term_acos+1