# -*- coding: utf-8 -*-

import akshare as ak
df = ak.stock_zh_a_spot_em()
df.to_csv('data.csv', header=True,index =False) 