# Call Prediction Model

filename = 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' 

reset
#plot 0 title "zero" \
        , filename using 1:2 '%lf,%lf,%lf,%lf' with lines title "p(w)" \
        , filename using 1:3 '%lf,%lf,%lf,%lf' with lines title "dp(w)/dw" \
        , filename using 1:4 '%lf,%lf,%lf,%lf' with lines title "_avgBlinds/fNRank"
#plot [0:0.5] 0 title "zero" \
        , filename using 1:2 '%lf,%lf,%lf,%lf' with lines title "p(w)" \
        , filename using 1:4 '%lf,%lf,%lf,%lf' with lines title "_avgBlinds/fNRank"
plot [0.5:1] 0 title "zero" \
        , filename using 1:2 '%lf,%lf,%lf,%lf' with lines title "p(w)" \
        , filename using 1:4 '%lf,%lf,%lf,%lf' with lines title "_avgBlinds/fNRank"
#plot [0.585:0.62] filename using 1:2 '%lf,%lf,%lf,%lf' with lines title "p(w)" , 0 title "zero"


