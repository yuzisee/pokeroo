# Call Prediction Model

chipcountFile = 'F:\X\Fross_G\Yuzisee\holdem\codeblocks\chipcount.csv' 

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

        
#Hand,i4,GearBotV,MultiBotV,DangerV,ComV,NormV,TrapV,AceV,SpaceV
plot chipcountFile using '%*lf,%lf' with lines linewidth 1 title "i4" \
   , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "NormV" \
   , chipcountFile using '%*lf,%*lf,%lf' with lines linewidth 5 title "GearBotV" \
   , chipcountFile using '%*lf,%*lf,%*lf,%lf' with lines linewidth 5 title "MultiBotV" \
   , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "AceV" \
   , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "TrapV" \
   , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 2 title "SpaceV" \
   , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "ComV" \
   , chipcountFile using '%*lf,%*lf,%*lf,%*lf,%lf' with lines linewidth 3 title "DangerV" \
        