from typing import *
import pandas as pd
import numpy as np

from fasttest_internal.core import Exchange


def exchangeToDataFrame(exchange: Exchange) -> pd.DataFrame:
    """Takes in row major exchange data and return multi-indexed DataFrame.
    The first level of the multi-index is the asset name and the second level is the column name.
    Input:
        .      0      1      2      3      4      5      6      7      8      9      10    11
    0    NaN    NaN  100.0  101.0  102.0  103.0  104.0  105.0  105.0  106.0    NaN   NaN
    1  101.0  101.5  100.0   99.0   98.0   97.0  101.0  101.5  101.0  101.5  103.0  96.0

    Output:
        .                    asset1        asset2
                        open  close   open  close
    2000-06-05 05:00:00    NaN    NaN  101.0  101.5
    2000-06-06 05:00:00  100.0  101.0  100.0   99.0
    2000-06-07 05:00:00  102.0  103.0   98.0   97.0
    2000-06-08 05:00:00  104.0  105.0  101.0  101.5
    2000-06-09 05:00:00  105.0  106.0  101.0  101.5
    2000-06-12 05:00:00    NaN    NaN  103.0   96.0

    Args:
        exchange (Exchange): _description_

    Returns:
        pd.DataFrame: _description_
    """
    index: List[int] = exchange.getTimestamps()
    asset_map: Dict[str, int] = exchange.getAssetMap()
    columns: List[str] = exchange.getColumns()
    data: np.ndarray = exchange.getData()
    dfs = []
    for asset_name, asset_index in asset_map.items():
        asset_data = data[asset_index, :].reshape(len(index), len(columns))
        df = pd.DataFrame(asset_data, index=index, columns=columns)
        df.index = pd.to_datetime(df.index, unit="ns")
        df.columns = pd.MultiIndex.from_product([[asset_name], columns])
        dfs.append(df)
    df = pd.merge(*dfs, left_index=True, right_index=True)
    return df
