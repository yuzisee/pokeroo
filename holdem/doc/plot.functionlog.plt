# Call Prediction Model

filename = 'F:\X\Fross_G\Yuzisee\holdem\codeblocks\NormV.functionlog.csv' 

reset
plot      [0:6] filename using 1:2 '%lf,%lf' with lines linewidth 2 title "purebluff"

#plot      filename using 1:2 '%lf,%*lf,%lf' with lines   title "dgain" \
        , filename using 1:2 '%lf,%lf' with lines linewidth 2 title "gain" \

#plot     filename using 1:2 '%lf,%lf' with lines linewidth 2  title "gain" \
        , filename using 1:2 '%lf,%*lf,%lf' with lines title "dgain" \
        , filename using 1:2 '%lf,%*lf,%*lf,%lf' with lines title "wch%" \
        , filename using 1:2 '%lf,%*lf,%*lf,%*lf,%lf' with lines title "dwch%"
        