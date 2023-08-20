echo yo
IF NOT "%WINCEREL%"=="1" goto end

copy "%_TGTCPU%\%WINCEDEBUG%\MCS9900Ce50.*" "%_FLATRELEASEDIR%"
copy "%_TGTCPU%\%WINCEDEBUG%\Mosiisr99x.*" "%_FLATRELEASEDIR%"


:end