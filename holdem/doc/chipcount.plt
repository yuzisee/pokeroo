# Call Prediction Model

chipcountFile = 'F:\X\Fross_G\Yuzisee\holdem\codeblocks\chipcount.csv' 

reset
set key center top horizontal 
plot    chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "AceV" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "TrapV" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "NormV," \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "ComV" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 1 title "SpaceIV" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "AceIV" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "TrapIV" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "NormIV" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "ComIV" \
        , chipcountFile using '%*lf,%*lf,%*lf,%lf' with lines linewidth 1 title "A3" \
        , chipcountFile using '%*lf,%*lf,%lf' with lines linewidth 1 title "X3" \
        , chipcountFile using '%*lf,%lf' with lines linewidth 1 title "i4"
        

