cd /d "%~dp0"

FOR %%i IN (%*) DO (
    ISP_MW --Haze_Removal_Retinex --TransferChar 8 --Ymode 1 --sigma 15 --sigma 120 --tMap_thr 0.001 --ALmax 1 --tMapMin 0.1 --tMapMax 1.2 --strength 0.8 --ppmode 3 --lower_thr 0.02 --upper_thr 0.01 --HistBins 1024 --debug 0 --tag .Haze_Removal_Retinex_Linear %%i
)

pause