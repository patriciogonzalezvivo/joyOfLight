/*
 *  JoyOfLight
 *
 *  Copyright 2010 Patricio Gonzalez Vivo http://www.patriciogonzalezvivo.com
 *	All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***********************************************************************/
#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	camWidth = 640;
	camHeight = 480;
	
	humbral = 195;
	hCycle = 250;
	
	vFlip = false;
	hFlip = true;
	
	counter = 0;
	
	ofSetWindowShape(camWidth, camHeight);
	
	videoIn.setVerbose(true);
	videoIn.initGrabber(camWidth,camHeight, GL_RGB);
	entrada.allocate(camWidth,camHeight);
	filtro.allocate(camWidth,camHeight);
	film.allocate(camWidth, camHeight);
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(0, 0, 0);
	videoIn.grabFrame();
	
	if (videoIn.isFrameNew()){
		entrada.setFromPixels(videoIn.getPixels(),camWidth,camHeight);
		entrada.mirror(vFlip,hFlip);
		filtro = entrada;
		filtro.threshold(humbral);
		updateFilm(filtro.getPixels());	}
	
	if (hCycle < 255) hCycle += 0.01;
	else hCycle = 0;
	
	color.setHue(hCycle);
	color.setHsb(hCycle, 255, 255);
}

void testApp::cleanFilm(){
	unsigned char * filmPixels = film.getPixels();
	
	int lenght = camWidth * camHeight * 3;
	
	for( int i=0; i < lenght; i++ ) filmPixels[i] = 0;
	
	film.setFromPixels( filmPixels, camWidth, camHeight );
}

void testApp::updateFilm(unsigned char *mskPixels){
	unsigned char * filmPixels = film.getPixels();
	
	for( int i=0; i<camWidth; i++ ){
		for( int j=0; j<camHeight; j++ ){
			int p = j * camWidth + i;	
			if ( mskPixels[p] > 0){
				filmPixels[p * 3 + 0] = color.r;
				filmPixels[p * 3 + 1] = color.g;
				filmPixels[p * 3 + 2] = color.b;
			}
		}
	}
	
	superFastBlur(filmPixels, camWidth,camHeight, 2 );
	
	film.setFromPixels( filmPixels, camWidth, camHeight );
}

void  testApp::superFastBlur(unsigned char *pix, int w, int h, int radius) {
	
	if (radius<1) return;
	int wm=w-1;
	int hm=h-1;
	int wh=w*h;
	int div=radius+radius+1;
	unsigned char *r=new unsigned char[wh];
	unsigned char *g=new unsigned char[wh];
	unsigned char *b=new unsigned char[wh];
	int rsum,gsum,bsum,x,y,i,p,p1,p2,yp,yi,yw;
	int *vMIN = new int[MAX(w,h)];
	int *vMAX = new int[MAX(w,h)];
	
	unsigned char *dv=new unsigned char[256*div];
	for (i=0;i<256*div;i++) dv[i]=(i/div);
	
	yw=yi=0;
	
	for (y=0;y<h;y++){
		rsum=gsum=bsum=0;
		for(i=-radius;i<=radius;i++){
			p = (yi + MIN(wm, MAX(i,0))) * 3;
			rsum += pix[p];
			gsum += pix[p+1];
			bsum += pix[p+2];
		}
		for (x=0;x<w;x++){
			
			r[yi]=dv[rsum];
			g[yi]=dv[gsum];
			b[yi]=dv[bsum];
			
			if(y==0){
				vMIN[x]=MIN(x+radius+1,wm);
				vMAX[x]=MAX(x-radius,0);
			} 
			p1 = (yw+vMIN[x])*3;
			p2 = (yw+vMAX[x])*3;
			
			rsum += pix[p1]      - pix[p2];
			gsum += pix[p1+1]   - pix[p2+1];
			bsum += pix[p1+2]   - pix[p2+2];
			
			yi++;
		}
		yw+=w;
	}
	
	for (x=0;x<w;x++){
		rsum=gsum=bsum=0;
		yp=-radius*w;
		for(i=-radius;i<=radius;i++){
			yi=MAX(0,yp)+x;
			rsum+=r[yi];
			gsum+=g[yi];
			bsum+=b[yi];
			yp+=w;
		}
		yi=x;
		for (y=0;y<h;y++){
			pix[yi*3]      = dv[rsum];
			pix[yi*3 + 1]   = dv[gsum];
			pix[yi*3 + 2]   = dv[bsum];
			if(x==0){
				vMIN[y]=MIN(y+radius+1,hm)*w;
				vMAX[y]=MAX(y-radius,0)*w;
			} 
			p1=x+vMIN[y];
			p2=x+vMAX[y];
			
			rsum+=r[p1]-r[p2];
			gsum+=g[p1]-g[p2];
			bsum+=b[p1]-b[p2];
			
			yi+=w;
		}
	}
	
	delete r;
	delete g;
	delete b;
	
	delete vMIN;
	delete vMAX;
	delete dv;
}

//--------------------------------------------------------------
void testApp::draw(){
	glDisable(GL_BLEND);
	ofSetColor(255,255,255);
	entrada.draw(0,0,ofGetWidth(),ofGetHeight());
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	film.draw(0,0,ofGetWidth(),ofGetHeight());
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	switch (key){
		case 'f':
			ofToggleFullscreen();
			break;
		case 'i':
			if(ofGetWindowMode() == OF_WINDOW) videoIn.videoSettings();
			break;
		case '+':
			humbral ++;
			if (humbral > 255) humbral = 255;
			break;
		case '-':
			humbral --;
			if (humbral < 0) humbral = 0;
			break;
		case 'v':
			vFlip = !vFlip;
			break;
		case 'h':
			hFlip = !hFlip;
			break;			
		case ' ':
			cleanFilm();
			break;
		case OF_KEY_RETURN:
			ofImage temp;
			temp.setFromPixels(film.getPixels(), camWidth, camHeight, OF_IMAGE_COLOR);
			temp.saveImage(ofToString(counter++)+".jpg", OF_IMAGE_QUALITY_BEST);
			break;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

