#include "StdAfx.h"
#include "Histogram.h"
#include <atlimage.h>
#include <AtlBase.h>
#include <AtlConv.h>

//#include "CImg.h"
//using namespace cimg_library;

/*ALM
CHistogram::CHistogram(void)
{

}

CHistogram::~CHistogram(void)
{
}
*//*ALM
int CHistogram::CreateImage(char *fileName, int nClasses, double *classVals, char * title, char *yLabel, char *xLabel, 
		char **xLabels, int width, int height)
{
	int ret = 0;
	/*HDC hdcMain = GetDC(NULL);

    BITMAPINFO bmInfo; 
    bmInfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biPlanes        = 1;
	bmInfo.bmiHeader.biBitCount      = 24;
	bmInfo.bmiHeader.biCompression   = BI_RGB;
	bmInfo.bmiHeader.biXPelsPerMeter = 0;
	bmInfo.bmiHeader.biYPelsPerMeter = 0;
	bmInfo.bmiHeader.biClrUsed       = 0;
	bmInfo.bmiHeader.biClrImportant  = 0;
	bmInfo.bmiHeader.biWidth         = Header.numeast;
	bmInfo.bmiHeader.biHeight        = Header.numnorth;
	bmInfo.bmiHeader.biSizeImage     = 0;

	PVOID pBits;
	HBITMAP hBitmap = CreateDIBSection(hdcMain, &bmInfo, DIB_RGB_COLORS, &pBits, NULL, 0);*
	CImage image;
	if(FAILED(image.Create(width, height, 24)))
	{
		return ret;
	}
	//figure y Extent for automatic labelling
	double ymax = 0.0, yStep = 10.0, maxPercent = 0.0;
	for(int i = 0; i < nClasses; i++)
		ymax = maxPercent = max(ymax, classVals[i]);
	//the above are always percent (for this varsion), range 0.0 - 100.0
	if(ymax > 60.0)
	{
		int ideal = ymax / 10 + ymax;
		yStep = 20.0;
		ymax = ideal;
	}
	else if(ymax > 35.0)
	{
		int ideal = ymax / 10 + ymax;
		ymax = ideal;
		yStep = 10.0;
	}
	else if(ymax > 15.0)
	{
		int ideal = ymax / 10 + ymax;
		ymax = ideal;
		yStep = 5.0;
	}
	else if(ymax > 5.0)
	{
		int ideal = ymax + 1;
		ymax = ideal;
		yStep = 2.0;
	}
	else if(ymax > 1.0)
	{
		int ideal = ymax + 1;
		ymax = ideal;
		yStep = 1.0;
	}
	else
	{
		yStep = 0.2;
		ymax = 1.0;
	}
	int x0, x1, y0, y1;
	x0 = width / 8;
	x1 = width - 10;
	y0 = height - height/5;
	y1 = height/10;
	double yFactor = (double(y0 - y1)) / double(ymax);
	CDC *pDC = CDC::FromHandle(image.GetDC());
	pDC->FillSolidRect(0, 0, width, height, RGB(255, 255, 255));
	CFont headerFont, axisFont, vheaderFont, vFont;
	headerFont.CreateFont( y1/ 2, x0/ 6, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 
		OUT_TT_PRECIS, CLIP_TT_ALWAYS, DEFAULT_QUALITY, VARIABLE_PITCH, _T("helvetica"));
	vheaderFont.CreateFont(x0 / 4,  y1 / 4 , 900, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 
		OUT_TT_PRECIS, CLIP_TT_ALWAYS, DEFAULT_QUALITY, VARIABLE_PITCH, _T("helvetica"));
	vFont.CreateFont(x0 / 6,  y1 / 6 , 900, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
		OUT_TT_PRECIS, CLIP_TT_ALWAYS, DEFAULT_QUALITY, VARIABLE_PITCH, _T("helvetica"));
	axisFont.CreateFont( y1/ 2, x0 / 8, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
		OUT_TT_PRECIS, CLIP_TT_ALWAYS, DEFAULT_QUALITY, VARIABLE_PITCH, _T("helvetica"));
	CPen axisPen, blackPen1;
	axisPen.CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
	blackPen1.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	CBrush grayBrush(RGB(125, 125, 125));
	CPen *oldPen = pDC->SelectObject(&axisPen);
	CFont *oldFont = pDC->SelectObject(&headerFont);
	CBrush *oldBrush = pDC->SelectObject(&grayBrush);
	pDC->SetTextAlign(TA_BASELINE | TA_CENTER);
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(x0 + (x1 - x0) / 2, y1 / 2 + 1, title);
	pDC->TextOut(x0 + (x1 - x0) / 2, y0 + (height - y0) / 3 * 2, xLabel);
	pDC->SelectObject(&vheaderFont);
	pDC->SetTextAlign(TA_BASELINE | TA_CENTER);
	pDC->TextOut(x0 / 3, (y1 + y0) / 2, yLabel);
	pDC->SelectObject(&axisFont);
	pDC->SetTextAlign(TA_BOTTOM | TA_LEFT);
	//char tmp[128];
	CString tmp;
	double barFactor = (y0 - y1) / ymax;
	pDC->MoveTo(x0, y1);
	pDC->LineTo(x0, y0);
	pDC->LineTo(x1, y0);
	int barWidth = (x1 - x0) / (nClasses);
	int l = x0;
	pDC->SelectObject(&blackPen1);
	for(int c = 0; c < nClasses; c++)
	{
		int top = y0 - barFactor * classVals[c];// - ymin);
		pDC->Rectangle(l, top, l + barWidth, y0);
		pDC->MoveTo(l, y0);
		pDC->LineTo(l, y0 + height/40);
			//tmp.Format(_T("%.0f"), ffMin + c * ffStep);
		pDC->SelectObject(&axisFont);
		pDC->SetTextAlign(TA_BOTTOM | TA_CENTER);
		pDC->SetTextColor(RGB(0, 0, 0));
		if(nClasses <= 11 || c % 2 == 0)
			pDC->TextOut(l + barWidth / 2, y0 + (height - y0) / 3, xLabels[c]);
		else
			pDC->TextOut(l + barWidth / 2, y0 + (height - y0) / 2 , xLabels[c]);
		l += barWidth;
	}
	//output y axis labels
	pDC->SelectObject(&axisFont);
	pDC->SetTextAlign(TA_BOTTOM | TA_RIGHT);
	pDC->SetTextColor(RGB(0, 0, 0));
	double y = 0.0;
	while(y <= maxPercent)
	{
		tmp.Format(_T("%.0f"), y);
		int ySpot = y0 - barFactor * y;//(y * (double)(y0 - y1) / maxPercent);
		pDC->TextOut(x0 - (x0 / 3), ySpot, tmp);
		pDC->MoveTo(x0, ySpot);
		pDC->LineTo(x0 - (x0 / 4), ySpot);
		y += yStep;
	}



	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldFont);
	image.ReleaseDC();
	CA2W pszA(fileName);
	//image.Save(pszA);
	image.Save(fileName);
	return 1;
}

/*int CHistogram::CreateImage(char *fileName, int nClasses, double *classVals, char * title, char *yLabel, char *xLabel, 
		char **xLabels, int width, int height)
{
	int ret = 0;
	
	CImg<unsigned char> img(width,height,1,3);

    const unsigned char gray[] = { 125,125,125 };
	const unsigned char black[] = {0,0,0};
	const unsigned char red[] = {153,153,153};
	const unsigned char white[] = {255,255,255};
	CImg<unsigned char> text;

	//figure y Extent for automatic labelling
	double ymax = 0.0, yStep = 10.0, maxPercent = 0.0;
	for(int i = 0; i < nClasses; i++)
		ymax = maxPercent = max(ymax, classVals[i]);
	//the above are always percent (for this varsion), range 0.0 - 100.0
	if(ymax > 60.0)
	{
		int ideal = ymax / 10 + ymax;
		yStep = 20.0;
		ymax = ideal;
	}
	else if(ymax > 35.0)
	{
		int ideal = ymax / 10 + ymax;
		ymax = ideal;
		yStep = 10.0;
	}
	else if(ymax > 15.0)
	{
		int ideal = ymax / 10 + ymax;
		ymax = ideal;
		yStep = 5.0;
	}
	else if(ymax > 5.0)
	{
		int ideal = ymax + 1;
		ymax = ideal;
		yStep = 2.0;
	}
	else if(ymax > 1.0)
	{
		int ideal = ymax + 1;
		ymax = ideal;
		yStep = 1.0;
	}
	else
	{
		yStep = 0.2;
		ymax = 1.0;
	}
	int x0, x1, y0, y1;
	x0 = width / 8;
	x1 = width - 10;
	y0 = height - height/5;
	y1 = height/10;
	double yFactor = (double(y0 - y1)) / double(ymax);
	
	img.fill(255,255,255);
	
	
	text.assign().draw_text(0,0,title,white,black,1,24);
	img.draw_image(x0 + (x1 - x0 - text.width())/2,0, ~text);
	
	text.assign().draw_text(0,0, xLabel, white,black,1,13);
	img.draw_image(x0 + (x1 - x0 - text.width())/2, y0 + (height - y0) / 3,  ~text);

	text.assign().draw_text(0,0,yLabel,white,black,1,13).rotate(-90);
	img.draw_image(2, (img.height() - text.height())/2,~text);
	

	
	
	
	double barFactor = (y0 - y1) / ymax;

	img.draw_line(x0,y1,x0,y0,gray);
	img.draw_line(x0,y0,x1,y0,gray);

	int barWidth = (x1 - x0) / (nClasses);
	int l = x0;
	
	for(int c = 0; c < nClasses; c++)
	{
		int top = y0 - barFactor * classVals[c];// - ymin);
	
		img.draw_rectangle(l,top,l+barWidth,y0,gray,1);
		
		img.draw_line(l,y0, l, y0 + height/40,black,1);

		text.assign().draw_text(0,0,xLabels[c],white,black,1,13);
		img.draw_image(l + barWidth / 2 -2, y0 + (height - y0) / 3 - 15,~text);
		l += barWidth;
	}

	//output y axis labels
	
	double y = 0.0;
	while(y <= maxPercent)
	{
		
		int ySpot = y0 - barFactor * y;//(y * (double)(y0 - y1) / maxPercent);
		

		text.assign().draw_text(0,0,"%d",white,black,1,13,(int) y);
		img.draw_image(x0 - (x0 / 3)- 8, ySpot - 7, ~text);
		
		img.draw_line(x0,ySpot,x0 - (x0 / 5), ySpot,black,1);

		y += yStep;
	}



	
	img.save_jpeg(fileName);
	return 1;
}*/
