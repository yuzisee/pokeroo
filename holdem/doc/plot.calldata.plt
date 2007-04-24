# Call Prediction Model

reset
plot 0 title "zero" \
        , 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' using 1:2 '%lf,%lf,%lf,%lf' with lines title "p(w)" \
        , 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' using 1:3 '%lf,%lf,%lf,%lf' with lines title "dp(w)/dw" \
        , 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' using 1:4 '%lf,%lf,%lf,%lf' with lines title "_avgBlinds/fNRank"
#plot [0:0.5] 0 title "zero" \
        , 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' using 1:2 '%lf,%lf,%lf,%lf' with lines title "p(w)" \
        , 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' using 1:4 '%lf,%lf,%lf,%lf' with lines title "_avgBlinds/fNRank"
plot [0.5:1] 0 title "zero" \
        , 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' using 1:2 '%lf,%lf,%lf,%lf' with lines title "p(w)" \
        , 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' using 1:4 '%lf,%lf,%lf,%lf' with lines title "_avgBlinds/fNRank"
#plot [0.585:0.62] 'F:\X\Fross_G\Yuzisee\holdem\vs2005\holdem\callmodel.csv' using 1:2 '%lf,%lf,%lf,%lf' with lines title "p(w)" , 0 title "zero"


