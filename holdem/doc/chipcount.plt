# Call Prediction Model

chipcountFile = 'C:\Users\Joseph\Documents\holdem.release\Results.16.05.2007-21.16.11.27\chipcount.csv' 

reset
set key center top horizontal 
#plot    chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "AceV" \
#        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "TrapV" \
#        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "NormV," \
#        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "ComV" \
#        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 1 title "SpaceIV" \
#        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "AceIV" \
#        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "TrapIV" \
#        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "NormIV" \
#        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "ComIV" \
#        , chipcountFile using '%*lf,%*lf,%*lf,%lf' with lines linewidth 1 title "A3" \
#        , chipcountFile using '%*lf,%*lf,%lf' with lines linewidth 1 title "X3" \
#        , chipcountFile using '%*lf,%lf' with lines linewidth 1 title "i4"

plot    chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 1 title "ActionBot" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "SpaceBot" \
        , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "NormalBot" \
        , chipcountFile using '%*lf,%*lf,%*lf,%lf' with lines linewidth 1 title "ConservativeBot" \
        , chipcountFile using '%*lf,%*lf,%lf' with lines linewidth 1 title "TrapBot" \
        , chipcountFile using '%*lf,%lf' with lines linewidth 5 title "Hero"
        

