HCTX static NEAR TabletInit(HWND hWnd, FIX32 scale[])
{

	/* get default region */
	WTInfo(WTI_DEFCONTEXT, 0, &lcMine);

	/* modify the digitizing region */
	strcpy(lcMine.lcName, "WacomRecord Digitizing");
	lcMine.lcOptions |= CXO_MESSAGES;
	lcMine.lcPktData = PACKETDATA;
	lcMine.lcPktMode = PACKETMODE;
	lcMine.lcMoveMask = PACKETDATA;
	lcMine.lcBtnUpMask = lcMine.lcBtnDnMask;

	/* output in 1000ths of cm */
	lcMine.lcOutOrgX = lcMine.lcOutOrgY = 0;
	lcMine.lcOutExtX = INT(scale[0] * lcMine.lcInExtX);
	lcMine.lcOutExtY = INT(scale[1] * lcMine.lcInExtY);

	/* open the region */
	return WTOpen(hWnd, &lcMine, TRUE);

}
/* -------------------------------------------------------------------------- */
/* return scaling factors in thousandths of cm per axis unit */
static void TabletScaling(FIX32 scale[])
{
	AXIS aXY[2];
	int i;
	UINT wDevice;

	/* get the data */
	WTInfo(WTI_DEFCONTEXT, CTX_DEVICE, &wDevice);
	WTInfo(WTI_DEVICES+wDevice, DVC_X, &aXY[0]);
	WTInfo(WTI_DEVICES+wDevice, DVC_Y, &aXY[1]);

	/* calculate the scaling factors */
	for (i = 0; i < 2; i++) {
		FIX_DIV(scale[i], CASTFIX32(1000), aXY[i].axResolution);
		if (aXY[i].axUnits == TU_INCHES) {
			FIX_MUL(scale[i], scale[i], Inch2Cm);
		}
	}
}
/* -------------------------------------------------------------------------- */
