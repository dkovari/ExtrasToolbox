function changeStackIndex(this,stackDim,newIndex)
%change index of specified stack dimension (called by gui controls)

FI = this.FrameIndicies;
FI(stackDim) = newIndex;
this.set_FrameIndicies(FI);