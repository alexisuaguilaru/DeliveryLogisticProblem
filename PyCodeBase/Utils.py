def ConvertRowToPoint(
        RowPoint: list[str],
    ) -> tuple[float,float]:

    return (
        float(RowPoint[0]),
        float(RowPoint[1]),
    )