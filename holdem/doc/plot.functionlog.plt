# Call Prediction Model

filename = 'C:\Users\Joseph\Documents\Yuzisee\holdem\codeblocks\ConservativeBotV.functionlog.purebluff.csv' 

reset
plot      [0:700] filename using 1:2 '%lf,%lf' with lines linewidth 2 title "purebluff"

#plot      filename using 1:2 '%lf,%*lf,%lf' with lines   title "dgain" \
        , filename using 1:2 '%lf,%lf' with lines linewidth 2 title "gain" \

#plot     filename using 1:2 '%lf,%lf' with lines linewidth 2  title "gain" \
        , filename using 1:2 '%lf,%*lf,%lf' with lines title "dgain" \
        , filename using 1:2 '%lf,%*lf,%*lf,%lf' with lines title "wch%" \
        , filename using 1:2 '%lf,%*lf,%*lf,%*lf,%lf' with lines title "dwch%"
        