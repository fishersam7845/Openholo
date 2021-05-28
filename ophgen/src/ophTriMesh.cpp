/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install, copy or use the software.
//
//
//                           License Agreement
//                For Open Source Digital Holographic Library
//
// Openholo library is free software;
// you can redistribute it and/or modify it under the terms of the BSD 2-Clause license.
//
// Copyright (C) 2017-2024, Korea Electronics Technology Institute. All rights reserved.
// E-mail : contact.openholo@gmail.com
// Web : http://www.openholo.org
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//  1. Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the copyright holder or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
// This software contains opensource software released under GNU Generic Public License,
// NVDIA Software License Agreement, or CUDA supplement to Software License Agreement.
// Check whether software you use contains licensed software.
//
//M*/

#include "ophTriMesh.h"
#include "tinyxml2.h"
#include "PLYparser.h"

int i = 0;
#define for_i(iter, oper)	for(i=0;i<iter;i++){oper}

#define _X1 0
#define _Y1 1
#define _Z1 2
#define _X2 3
#define _Y2 4
#define _Z2 5
#define _X3 6
#define _Y3 7
#define _Z3 8

void ophTri::setMode(bool isCPU)
{
	is_CPU = isCPU;
}

void ophTri::setViewingWindow(bool is_ViewingWindow)
{
	this->is_ViewingWindow = is_ViewingWindow;
}

bool ophTri::loadMeshText(const char* fileName) {

	cout << "Mesh Text File Load..." << endl;

	ifstream file;
	file.open(fileName);

	if (!file) {
		cout << "Open failed - no such file" << endl;
		cin.get();
		return false;
	}

	triMeshArray = new Real[9 * 10000];

	Real data;
	uint num_data;

	num_data = 0;
	do {
		file >> data;
		triMeshArray[num_data] = data;
		num_data++;
	} while (file.get() != EOF);

	meshData->n_faces = num_data / 9;
	triMeshArray[meshData->n_faces * 9] = EOF;

	return true;
}

bool ophTri::loadMeshData(const char* fileName, const char* ext) {
	meshData = new OphMeshData;
	cout << "ext = " << ext << endl;

	if (!strcmp(ext, "txt")) {
		cout << "Text File.." << endl;
		if (loadMeshText(fileName))
			cout << "Mesh Data Load Finished.." << endl;
		else
			cout << "Mesh Data Load Failed.." << endl;
	}
	else if (!strcmp(ext, "ply")) {
		cout << "PLY File.." << endl;
		PLYparser meshPLY;
		if (meshPLY.loadPLY(fileName, meshData->n_faces, meshData->color_channels, &meshData->face_idx, &meshData->vertex, &meshData->color))
			cout << "Mesh Data Load Finished.." << endl;
		else
		{
			cout << "Mesh Data Load Failed.." << endl;
			return false;
		}
	}
	else {
		cout << "Error: Mesh data must be .txt or .ply" << endl;
		return false;
	}
	meshData->n_faces /= 3;
	triMeshArray = meshData->vertex;

	return true;
}

bool ophTri::readConfig(const char* fname)
{
	if (!ophGen::readConfig(fname))
		return false;

	cout << "wavelength = " << context_.wave_length[0] << endl;
	cout << "pixNum = " << context_.pixel_number[_X] << ", " << context_.pixel_number[_Y] << endl;
	cout << "pixPit = " << context_.pixel_pitch[_X] << ", " << context_.pixel_pitch[_Y] << endl;
	LOG("Reading....%s...", fname);

	auto start = CUR_TIME;

	using namespace tinyxml2;
	/*XML parsing*/
	tinyxml2::XMLDocument xml_doc;
	XMLNode *xml_node;

	if (!checkExtension(fname, ".xml"))
	{
		LOG("file's extension is not 'xml'\n");
		return false;
	}
	if (xml_doc.LoadFile(fname) != XML_SUCCESS)
	{
		LOG("Failed to load file \"%s\"\n", fname);
		return false;
	}
	xml_node = xml_doc.FirstChild();

	// about object
	auto next = xml_node->FirstChildElement("ScaleX");
	if (!next || XML_SUCCESS != next->QueryDoubleText(&objSize[_X]))
		return false;
	next = xml_node->FirstChildElement("ScaleY");
	if (!next || XML_SUCCESS != next->QueryDoubleText(&objSize[_Y]))
		return false;
	next = xml_node->FirstChildElement("ScaleZ");
	if (!next || XML_SUCCESS != next->QueryDoubleText(&objSize[_Z]))
		return false;

	next = xml_node->FirstChildElement("LampDirectionX");
	if (!next || XML_SUCCESS != next->QueryDoubleText(&illumination[_X]))
		return false;
	next = xml_node->FirstChildElement("LampDirectionY");
	if (!next || XML_SUCCESS != next->QueryDoubleText(&illumination[_Y]))
		return false;
	next = xml_node->FirstChildElement("LampDirectionZ");
	if (!next || XML_SUCCESS != next->QueryDoubleText(&illumination[_Z]))
		return false;

	// about extra functions
	next = xml_node->FirstChildElement("Random_Phase");
	if (!next || XML_SUCCESS != next->QueryBoolText(&randPhase)) {
		LOG("\n\nPut Random_Phase in Config file\n");  return false;
	}

	next = xml_node->FirstChildElement("Occlusion");
	if (!next || XML_SUCCESS != next->QueryBoolText(&occlusion)) {
		LOG("\n\nPut Occlusion in Config file\n");  return false;
	}
	next = xml_node->FirstChildElement("Texture");
	if (!next || XML_SUCCESS != next->QueryBoolText(&textureMapping)) {
		LOG("\n\nPut Texture in Config file\n");  return false;
	}
	if (textureMapping == true) {
		//next = xml_node->FirstChildElement("TextureDimX");
		//if (!next || XML_SUCCESS != next->QueryDoubleText(&texture.dim[_X])) {
		//	LOG("\n\nPut TextureDimX in Config file\n");  return false;
		//}
		//next = xml_node->FirstChildElement("TextureDimY");
		//if (!next || XML_SUCCESS != next->QueryDoubleText(&texture.dim[_Y])) {
		//	LOG("\n\nPut TextureDimY in Config file\n");  return false;
		//}
		next = xml_node->FirstChildElement("TexturePeriod");
		if (!next || XML_SUCCESS != next->QueryDoubleText(&texture.period)) {
			LOG("\n\nPut TexturePeriod in Config file\n");  return false;
		}
	}
	auto end = CUR_TIME;
	auto during = ((chrono::duration<Real>)(end - start)).count();
	LOG("%lf (s).. Config Load Finished...\n", during);
	initialize();
	return true;
}


void ophTri::loadTexturePattern(const char* fileName, const char* ext) {

	uchar* image;
	image = loadAsImg(fileName);

	int bytesperpixel;
	int size[2] = { 0,0 };
	getImgSize(texture.dim[_X], texture.dim[_Y], bytesperpixel, fileName);
	cout << "texture : " << texture.dim[0] << ", " << texture.dim[1] << endl;
	
	texture.freq = 1 / texture.period;

	texture.pattern = new Complex<Real>[texture.dim[_X] * texture.dim[_Y]];
	textFFT = new Complex<Real>[texture.dim[_X] * texture.dim[_Y]];
	fft2(texture.dim, texture.pattern, OPH_FORWARD, OPH_ESTIMATE);
	fftExecute(texture.pattern);
	fftwShift(texture.pattern, textFFT, texture.dim[_X], texture.dim[_Y], OPH_FORWARD);

	tempFreqTermX = new Real[context_.pixel_number[_X] * context_.pixel_number[_Y]];
	tempFreqTermY = new Real[context_.pixel_number[_X] * context_.pixel_number[_Y]];

}


void ophTri::initializeAS()
{
	if (angularSpectrum) {
		delete[] angularSpectrum;
		angularSpectrum = nullptr;
	}
	angularSpectrum = new Complex<Real>[pnXY];
	memset(angularSpectrum, 0, sizeof(Complex<Real>) * pnXY);
}


void ophTri::objNormCenter()
{
	if (normalizedMeshData) {
		delete[] normalizedMeshData;
		normalizedMeshData = nullptr;
	}

	int nFace = meshData->n_faces;

	normalizedMeshData = new Real[nFace * 9];

	Real* x_point = new Real[nFace * 3];
	Real* y_point = new Real[nFace * 3];
	Real* z_point = new Real[nFace * 3];

	int i;
#ifdef _OPENMP
#pragma omp for private(i)
#endif
	for (i = 0; i < nFace * 3; i++) {
		int idx = i * 3;
		x_point[i] = triMeshArray[idx + _X];
		y_point[i] = triMeshArray[idx + _Y];
		z_point[i] = triMeshArray[idx + _Z];
	}

	Real x_cen = (maxOfArr(x_point, nFace * 3) + minOfArr(x_point, nFace * 3)) / 2;
	Real y_cen = (maxOfArr(y_point, nFace * 3) + minOfArr(y_point, nFace * 3)) / 2;
	Real z_cen = (maxOfArr(z_point, nFace * 3) + minOfArr(z_point, nFace * 3)) / 2;

	Real* centered = new Real[nFace * 9];

#ifdef _OPENMP
#pragma omp for private(i)
#endif
	for (i = 0; i < nFace * 3; i++) {
		int idx = i * 3;
		centered[idx + _X] = x_point[i] - x_cen;
		centered[idx + _Y] = y_point[i] - y_cen;
		centered[idx + _Z] = z_point[i] - z_cen;
	}

	Real x_del = (maxOfArr(x_point, nFace * 3) - minOfArr(x_point, nFace * 3));
	Real y_del = (maxOfArr(y_point, nFace * 3) - minOfArr(y_point, nFace * 3));
	Real z_del = (maxOfArr(z_point, nFace * 3) - minOfArr(z_point, nFace * 3));
	Real del = maxOfArr({ x_del, y_del, z_del });

#if 1
#ifdef _OPENMP
#pragma omp for private(i)
#endif
	for (i = 0; i < nFace * 9; i++) {
		normalizedMeshData[i] = centered[i] / del;
	}
#else
	for_i(nFace * 9,
		*(normalizedMeshData + i) = *(centered + i) / del;
	);
#endif
	delete[] centered, x_point, y_point, z_point;
}


void ophTri::objScaleShift()
{
	scaledMeshData = new Real[meshData->n_faces * 9];

	objNormCenter();
	Real *pMesh = nullptr;

	if (is_ViewingWindow) {
		pMesh = new Real[meshData->n_faces * 9];
		transVW(meshData->n_faces * 9, pMesh, normalizedMeshData);
	}
	else {
		pMesh = normalizedMeshData;
	}

	int i;
#ifdef _OPENMP
	int num_threads;
#pragma omp parallel
	{
		num_threads = omp_get_num_threads(); // get number of Multi Threading
#pragma omp for private(i)
#endif
		for (i = 0; i < meshData->n_faces * 3; i++) {
			int idx = i * 3;
			Real pcx = pMesh[idx + _X];
			Real pcy = pMesh[idx + _Y];
			Real pcz = pMesh[idx + _Z];

			scaledMeshData[idx + _X] = pcx * objSize[_X] + context_.shift[_X];
			scaledMeshData[idx + _Y] = pcy * objSize[_Y] + context_.shift[_Y];
			scaledMeshData[idx + _Z] = pcz * objSize[_Z] + context_.shift[_Z];
		}
	}
	if (is_ViewingWindow) {
		delete[] pMesh;
	}
	delete[] normalizedMeshData;

	cout << "Object Scaling and Shifting Finishied.." << endl;

#ifdef _OPENMP
	cout << ">>> All " << num_threads << " threads" << endl;
#endif
}

void ophTri::objSort()
{
	Real* centerZ = new Real[meshData->n_faces];
	for_i(meshData->n_faces,
		centerZ[i] = (scaledMeshData[i * 9 + _Z1] + scaledMeshData[i * 9 + _Z2] + scaledMeshData[i * 9 + _Z3]) / 3;
	);
	Real tempZ;
	Real tempM[9] = { 0,0,0,0,0,0,0,0,0 };
	int count;
	while (1) {
		for (int i = 0; i < meshData->n_faces - 1; i++) {
			count = 0;
			if (centerZ[i] < centerZ[i + 1]) {

				tempZ = centerZ[i];
				centerZ[i] = centerZ[i + 1];
				centerZ[i + 1] = tempZ;

				tempM[_X1] = scaledMeshData[i * 9 + _X1];	tempM[_Y1] = scaledMeshData[i * 9 + _Y1];	tempM[_Z1] = scaledMeshData[i * 9 + _Z1];
				tempM[_X2] = scaledMeshData[i * 9 + _X2];	tempM[_Y2] = scaledMeshData[i * 9 + _Y2];	tempM[_Z2] = scaledMeshData[i * 9 + _Z2];
				tempM[_X3] = scaledMeshData[i * 9 + _X3];	tempM[_Y3] = scaledMeshData[i * 9 + _Y3];	tempM[_Z3] = scaledMeshData[i * 9 + _Z3];

				scaledMeshData[i * 9 + _X1] = scaledMeshData[(i + 1) * 9 + _X1];	scaledMeshData[i * 9 + _Y1] = scaledMeshData[(i + 1) * 9 + _Y1];	scaledMeshData[i * 9 + _Z1] = scaledMeshData[(i + 1) * 9 + _Z1];
				scaledMeshData[i * 9 + _X2] = scaledMeshData[(i + 1) * 9 + _X2];	scaledMeshData[i * 9 + _Y2] = scaledMeshData[(i + 1) * 9 + _Y2];	scaledMeshData[i * 9 + _Z1] = scaledMeshData[(i + 1) * 9 + _Z2];
				scaledMeshData[i * 9 + _X3] = scaledMeshData[(i + 1) * 9 + _X3];	scaledMeshData[i * 9 + _Y3] = scaledMeshData[(i + 1) * 9 + _Y3];	scaledMeshData[i * 9 + _Z1] = scaledMeshData[(i + 1) * 9 + _Z3];

				scaledMeshData[(i + 1) * 9 + _X1] = tempM[_X1];	scaledMeshData[(i + 1) * 9 + _Y1] = tempM[_Y1];	scaledMeshData[(i + 1) * 9 + _Z1] = tempM[_Z1];
				scaledMeshData[(i + 1) * 9 + _X2] = tempM[_X2];	scaledMeshData[(i + 1) * 9 + _Y2] = tempM[_Y2];	scaledMeshData[(i + 1) * 9 + _Z2] = tempM[_Z2];
				scaledMeshData[(i + 1) * 9 + _X3] = tempM[_X3];	scaledMeshData[(i + 1) * 9 + _Y3] = tempM[_Y3];	scaledMeshData[(i + 1) * 9 + _Z3] = tempM[_Z3];

				count++;
			}
		}
		if (count == 0)
			break;
	}


}


void ophTri::objScaleShift(vec3 objSize_, vec3 objShift_)
{
	setObjSize(objSize_);
	setObjShift(objShift_);

	scaledMeshData = new Real[meshData->n_faces * 9];

	objNormCenter();
	Real *pMesh = nullptr;

	if (is_ViewingWindow) {
		pMesh = new Real[meshData->n_faces * 9];
		transVW(meshData->n_faces * 9, pMesh, normalizedMeshData);
	}
	else {
		pMesh = normalizedMeshData;
	}
	int i;

#ifdef _OPENMP
	int num_threads;
#pragma omp parallel
	{
		num_threads = omp_get_num_threads(); // get number of Multi Threading
#endif
		Real pcx, pcy, pcz;
		for (i = 0; i < meshData->n_faces * 3; i++) {
			pcx = *(pMesh + 3 * i + _X);
			pcy = *(pMesh + 3 * i + _Y);
			pcz = *(pMesh + 3 * i + _Z);

			*(scaledMeshData + 3 * i + _X) = pcx * objSize[_X] + context_.shift[_X];
			*(scaledMeshData + 3 * i + _Y) = pcy * objSize[_Y] + context_.shift[_Y];
			*(scaledMeshData + 3 * i + _Z) = pcz * objSize[_Z] + context_.shift[_Z];
		}
#ifdef _OPENMP
	}
	cout << ">>> All " << num_threads << " threads" << endl;
#endif
	if (is_ViewingWindow) {
		delete[] pMesh;
	}
	delete[] normalizedMeshData;
	cout << "Object Scaling and Shifting Finishied.." << endl;
}

vec3 vecCross(const vec3& a, const vec3& b)
{
	vec3 c;

	c(0) = a(0 + 1) * b(0 + 2) - a(0 + 2) * b(0 + 1);

	c(1) = a(1 + 1) * b(1 + 2) - a(1 + 2) * b(1 + 1);

	c(2) = a(2 + 1) * b(2 + 2) - a(2 + 2) * b(2 + 1);


	return c;
}


void ophTri::triTimeMultiplexing(char* dirName, uint ENCODE_METHOD, Real cenFx, Real cenFy, Real rangeFx, Real rangeFy, Real stepFx, Real stepFy) {

	TM = true;
	char strFxFy[30];
	Complex<Real>* AS = new Complex<Real>[context_.pixel_number[_X] * context_.pixel_number[_Y]];

	int nFx = floor(rangeFx / stepFx);
	int nFy = floor(rangeFy / stepFy);
	Real tFx, tFy, tFz;
	for (int iFy = 0; iFy <= nFy; iFy++) {
		for (int iFx = 0; iFx <= nFx; iFx++) {

			tFx = cenFx - rangeFx / 2 + iFx*stepFx;
			tFy = cenFy - rangeFy / 2 + iFy*stepFy;
			tFz = sqrt(1.0 / context_.wave_length[0] / context_.wave_length[0] - tFx*tFx - tFy*tFy);

			carrierWave[_X] = tFx*context_.wave_length[0];
			carrierWave[_Y] = tFy*context_.wave_length[0];
			carrierWave[_Z] = tFz*context_.wave_length[0];

			generateHologram(SHADING_FLAT);

			setEncodeMethod(ENCODE_METHOD);
			encoding();
			normalize();
			sprintf(strFxFy, "%s/holo_%d,%d.bmp", dirName, (int)tFx, (int)tFy);
			save(strFxFy, 8, nullptr, m_vecEncodeSize[_X], m_vecEncodeSize[_Y]);

			fft2(context_.pixel_number, complex_H[0], OPH_FORWARD);
			fftwShift(complex_H[0], complex_H[0], context_.pixel_number[_X], context_.pixel_number[_Y], OPH_FORWARD);

			setEncodeMethod(ENCODE_AMPLITUDE);
			encoding();
			normalize();
			sprintf(strFxFy, "%s/AS_%d,%d.bmp", dirName, (int)tFx, (int)tFy);
			save(strFxFy, 8, nullptr, m_vecEncodeSize[_X], m_vecEncodeSize[_Y]);


		}
	}
}

bool ophTri::generateHologram(uint SHADING_FLAG)
{
	//initialize();
	resetBuffer();

	auto start_time = CUR_TIME;
	LOG("1) Algorithm Method : Tri Mesh\n");
	LOG("2) Generate Hologram with %s\n", is_CPU ?
#ifdef _OPENMP
		"Multi Core CPU" :
#else
		"Single Core CPU" :
#endif
		"GPU");
	LOG("3) Transform Viewing Window : %s\n", is_ViewingWindow ? "ON" : "OFF");

	auto start = CUR_TIME;
	objScaleShift();
	objSort();
	pnX = context_.pixel_number[_X];
	pnY = context_.pixel_number[_Y];
	pnXY = pnX * pnY;

	(is_CPU) ? initializeAS() : initialize_GPU();
	(is_CPU) ? generateAS(SHADING_FLAG) : generateAS_GPU(SHADING_FLAG);
	cout << "AS generated?" << endl;
	if (is_CPU) {
		fft2(context_.pixel_number, angularSpectrum, OPH_BACKWARD, OPH_ESTIMATE);
		fftwShift(angularSpectrum, *(complex_H), context_.pixel_number[_X], context_.pixel_number[_Y], OPH_BACKWARD);
	}
	m_nProgress = 0;
	auto end = CUR_TIME;
	m_elapsedTime = ((std::chrono::duration<Real>)(end - start)).count();

	LOG("Total Elapsed Time: %lf (s)\n", m_elapsedTime);

	return true;
}

//bool ophTri::generateMeshHologram() {
//	cout << "Hologram Generation ..." << endl;
//	auto start = CUR_TIME;
//	resetBuffer();
//	initializeAS();
//	generateAS(SHADING_TYPE);
//
//	fft2(context_.pixel_number, angularSpectrum, OPH_BACKWARD, OPH_ESTIMATE);
//	fftwShift(angularSpectrum, (*complex_H), context_.pixel_number[_X], context_.pixel_number[_Y], OPH_BACKWARD);
//
//	fftFree();
//
//	auto end = CUR_TIME;
//	auto during = ((std::chrono::duration<Real>)(end - start)).count();
//
//	LOG("Total Elapsed Time: %lf (sec)\n", during);
//
//	return true;
//}


bool ophTri::generateAS(uint SHADING_FLAG)
{
	int N = meshData->n_faces;

	gFreq = new Real*[3];
	lFreq = new Real*[3];
	for (int i = 0; i < 3; i++) {
		gFreq[i] = new Real[pnXY];
		lFreq[i] = new Real[pnXY];
	}

	calGlobalFrequency();


	freqTermX = new Real[pnXY];		// used in every 'for' loop
	freqTermY = new Real[pnXY];		// used in every 'for' loop

	flxShifted = new Real[pnXY];
	flyShifted = new Real[pnXY];
	invLoRot = new Real[pnXY];

	refAS = new Complex<Real>[pnXY];

	phaseTerm = new Complex<Real>[pnXY];
	convol = new Complex<Real>[pnXY];
	rearAS = new Complex<Real>[pnXY];

	findNormals(SHADING_FLAG);

	k = 1 / context_.wave_length[0];
	kk = k * k;

	for (int j = 0; j < N; j++) {
		memcpy(mesh, &scaledMeshData[9 * j], sizeof(Real) * 9);

		if (!checkValidity(mesh, no[j]))
			continue;
		if (!findGeometricalRelations(mesh, no[j]))
			continue;
		if (!calFrequencyTerm())
			continue;
		switch (SHADING_FLAG)
		{
		case SHADING_FLAT:
			refAS_Flat(no[j]);
			break;
		case SHADING_CONTINUOUS:
			refAS_Continuous(j);
			break;
		default:
			LOG("error: WRONG SHADING_FLAG\n");
			return false;
		}
		if (!refToGlobal())
			continue;

		m_nProgress = (int)((Real)j * 100 / ((Real)N));
		char szLog[MAX_PATH];
		sprintf_s(szLog, "%d / %llu\n", j + 1, N);
		LOG(szLog);
	}
	LOG("Angular Spectrum Generated...\n");

	delete[] scaledMeshData, freqTermX, freqTermY, refAS, phaseTerm, convol;
	return true;
}

void ophTri::calGlobalFrequency()
{
	const Real ppX = context_.pixel_pitch[_X];
	const Real ppY = context_.pixel_pitch[_Y];
	const Real ssX = context_.ss[_X] = pnX * ppX;
	const Real ssY = context_.ss[_Y] = pnY * ppY;
	const uint nChannel = context_.waveNum;

	dfx = 1 / ssX;
	dfy = 1 / ssY;
	uint i = 0;
	Real dfl;

	for (uint ch = 0; ch < nChannel; ch++) {
		dfl = 1 / context_.wave_length[ch];
		for (int idxFy = pnY / 2; idxFy > -pnY / 2; idxFy--) {
			for (int idxFx = -pnX / 2; idxFx < pnX / 2; idxFx++) {

				gFreq[_X][i] = idxFx * dfx;
				gFreq[_Y][i] = idxFy * dfy;
				gFreq[_Z][i] = sqrt((dfl*dfl) - (gFreq[_X][i] * gFreq[_X][i]) - (gFreq[_Y][i] * gFreq[_Y][i]));

				i++;

			}
		}
	}

}

bool ophTri::findNormals(uint SHADING_FLAG)
{
	no = new vec3[meshData->n_faces];
	na = new vec3[meshData->n_faces];
	nv = new vec3[meshData->n_faces * 3];

	int num;

	for (num = 0; num < meshData->n_faces; num++)
	{
		*(no + num) = vecCross({ scaledMeshData[num * 9 + _X1] - scaledMeshData[num * 9 + _X2],
			scaledMeshData[num * 9 + _Y1] - scaledMeshData[num * 9 + _Y2],
			scaledMeshData[num * 9 + _Z1] - scaledMeshData[num * 9 + _Z2] },
			{ scaledMeshData[num * 9 + _X3] - scaledMeshData[num * 9 + _X2],
			scaledMeshData[num * 9 + _Y3] - scaledMeshData[num * 9 + _Y2],
			scaledMeshData[num * 9 + _Z3] - scaledMeshData[num * 9 + _Z2] });
	}

	Real normNo = 0.0;

	for (num = 0; num < meshData->n_faces; num++) {
		normNo += norm(no[num])*norm(no[num]);
	}

	normNo = sqrt(normNo);

	for (num = 0; num < meshData->n_faces; num++) {
		*(na + num) = no[num] / normNo;
	}
	for (num = 0; num < meshData->n_faces; num++) {
		normNo += norm(no[num])*norm(no[num]);
	}

	normNo = sqrt(normNo);

	if (SHADING_FLAG == SHADING_CONTINUOUS) {
		vec3* vertices = new vec3[meshData->n_faces * 3];
		vec3 zeros(0, 0, 0);

		for (uint idx = 0; idx < meshData->n_faces * 3; idx++) {
			memcpy(&vertices[idx], &scaledMeshData[idx * 3], sizeof(vec3));
		}
		for (uint idx1 = 0; idx1 < meshData->n_faces * 3; idx1++) {
			if (*(vertices + idx1) == zeros)
				continue;
			vec3 sum = *(na + idx1 / 3);
			uint count = 1;
			uint* idxes = new uint[meshData->n_faces * 3];
			*(idxes) = idx1;
			for (uint idx2 = idx1 + 1; idx2 < meshData->n_faces * 3; idx2++) {
				if (*(vertices + idx2) == zeros)
					continue;
				if ((vertices[idx1][0] == vertices[idx2][0])
					& (vertices[idx1][1] == vertices[idx2][1])
					& (vertices[idx1][2] == vertices[idx2][2])) {
					sum += *(na + idx2 / 3);
					*(vertices + idx2) = zeros;
					*(idxes + count) = idx2;
					count++;
				}
			}
			*(vertices + idx1) = zeros;

			sum = sum / count;
			sum = sum / norm(sum);
			for (uint i = 0; i < count; i++)
				*(nv + *(idxes + i)) = sum;

			delete[] idxes;
		}

		delete[] vertices;
	}

	return true;
}

bool ophTri::checkValidity(Real* mesh, vec3 no) {

	if (no[_Z] < 0 || (no[_X] == 0 && no[_Y] == 0 && no[_Z] == 0)) {
		return false;
	}
	if (no[_Z] >= 0)
		return true;

	return 0;
}

bool ophTri::findGeometricalRelations(Real* mesh, vec3 no)
{
	n = no / norm(no);
	if (n[_X] == 0 && n[_Z] == 0)
		th = 0;
	else
		th = atan(n[_X] / n[_Z]);

	temp = n[_Y] / sqrt(n[_X] * n[_X] + n[_Z] * n[_Z]);
	ph = atan(temp);
	geom.glRot[0] = cos(th);			geom.glRot[1] = 0;			geom.glRot[2] = -sin(th);
	geom.glRot[3] = -sin(ph)*sin(th);	geom.glRot[4] = cos(ph);	geom.glRot[5] = -sin(ph)*cos(th);
	geom.glRot[6] = cos(ph)*sin(th);	geom.glRot[7] = sin(ph);	geom.glRot[8] = cos(ph)*cos(th);

	for_i(3,
		mesh_local[3 * i] = geom.glRot[0] * mesh[3 * i] + geom.glRot[1] * mesh[3 * i + 1] + geom.glRot[2] * mesh[3 * i + 2];
	mesh_local[3 * i + 1] = geom.glRot[3] * mesh[3 * i] + geom.glRot[4] * mesh[3 * i + 1] + geom.glRot[5] * mesh[3 * i + 2];
	mesh_local[3 * i + 2] = geom.glRot[6] * mesh[3 * i] + geom.glRot[7] * mesh[3 * i + 1] + geom.glRot[8] * mesh[3 * i + 2];
	);

	geom.glShift[_X] = -mesh_local[_X1];
	geom.glShift[_Y] = -mesh_local[_Y1];
	geom.glShift[_Z] = -mesh_local[_Z1];

	for_i(3,
		mesh_local[3 * i] += geom.glShift[_X];
	mesh_local[3 * i + 1] += geom.glShift[_Y];
	mesh_local[3 * i + 2] += geom.glShift[_Z];
	);

	if (mesh_local[_X3] * mesh_local[_Y2] == mesh_local[_Y3] * mesh_local[_X2])
		return false;

	geom.loRot[0] = (refTri[_X3] * mesh_local[_Y2] - refTri[_X2] * mesh_local[_Y3]) / (mesh_local[_X3] * mesh_local[_Y2] - mesh_local[_Y3] * mesh_local[_X2]);
	geom.loRot[1] = (refTri[_X3] * mesh_local[_X2] - refTri[_X2] * mesh_local[_X3]) / (-mesh_local[_X3] * mesh_local[_Y2] + mesh_local[_Y3] * mesh_local[_X2]);
	geom.loRot[2] = (refTri[_Y3] * mesh_local[_Y2] - refTri[_Y2] * mesh_local[_Y3]) / (mesh_local[_X3] * mesh_local[_Y2] - mesh_local[_Y3] * mesh_local[_X2]);
	geom.loRot[3] = (refTri[_Y3] * mesh_local[_X2] - refTri[_Y2] * mesh_local[_X3]) / (-mesh_local[_X3] * mesh_local[_Y2] + mesh_local[_Y3] * mesh_local[_X2]);


	if ((geom.loRot[0] * geom.loRot[3] - geom.loRot[1] * geom.loRot[2]) == 0)
		return false;

	return true;
}


bool ophTri::calFrequencyTerm()
{
	int i;

	for (i = 0; i < pnXY; i++) {
		lFreq[_X][i] = geom.glRot[0] * gFreq[_X][i] + geom.glRot[1] * gFreq[_Y][i] + geom.glRot[2] * gFreq[_Z][i];
		lFreq[_Y][i] = geom.glRot[3] * gFreq[_X][i] + geom.glRot[4] * gFreq[_Y][i] + geom.glRot[5] * gFreq[_Z][i];
		lFreq[_Z][i] = sqrt(kk - lFreq[_X][i] * lFreq[_X][i] - lFreq[_Y][i] * lFreq[_Y][i]);

		flxShifted[i] = lFreq[_X][i] - k * (geom.glRot[0] * carrierWave[_X] + geom.glRot[1] * carrierWave[_Y] + geom.glRot[2] * carrierWave[_Z]);
		flyShifted[i] = lFreq[_Y][i] - k * (geom.glRot[3] * carrierWave[_X] + geom.glRot[4] * carrierWave[_Y] + geom.glRot[5] * carrierWave[_Z]);

	}

	det = geom.loRot[0] * geom.loRot[3] - geom.loRot[1] * geom.loRot[2];

	invLoRot[0] = (1 / det)*geom.loRot[3];
	invLoRot[1] = -(1 / det)*geom.loRot[2];
	invLoRot[2] = -(1 / det)*geom.loRot[1];
	invLoRot[3] = (1 / det)*geom.loRot[0];

#ifdef _OPENMP
#pragma omp for private(i)
#endif
	for (i = 0; i < pnXY; i++) {
		freqTermX[i] = invLoRot[0] * flxShifted[i] + invLoRot[1] * flyShifted[i];
		freqTermY[i] = invLoRot[2] * flxShifted[i] + invLoRot[3] * flyShifted[i];
	}

	return true;
}

bool ophTri::refAS_Flat(vec3 no)
{
	memset(refAS, 0, sizeof(Complex<Real>)*pnXY);

	refTerm1(0, 0);		// initialize at every loop
	refTerm2(0, 0);
	shadingFactor(0, 0);


	term1(0, 0);
	term1[_IM] = -2 * M_PI / context_.wave_length[0] * (
		carrierWave[_X] * (geom.glRot[0] * geom.glShift[_X] + geom.glRot[3] * geom.glShift[_Y] + geom.glRot[6] * geom.glShift[_Z])
		+ carrierWave[_Y] * (geom.glRot[1] * geom.glShift[_X] + geom.glRot[4] * geom.glShift[_Y] + geom.glRot[7] * geom.glShift[_Z])
		+ carrierWave[_Z] * (geom.glRot[2] * geom.glShift[_X] + geom.glRot[5] * geom.glShift[_Y] + geom.glRot[8] * geom.glShift[_Z]));
	if (illumination[_X] == 0 && illumination[_Y] == 0 && illumination[_Z] == 0) {
		shadingFactor = exp(term1);
	}
	else {
		normIllu = illumination / norm(illumination);
		shadingFactor = (2 * (n[_X] * normIllu[_X] + n[_Y] * normIllu[_Y] + n[_Z] * normIllu[_Z]) + 0.3)*exp(term1);
		if (shadingFactor[_RE] * shadingFactor[_RE] + shadingFactor[_IM] * shadingFactor[_IM] < 0)
			shadingFactor = 0;
	}

	if (occlusion == true) {
		memset(rearAS, 0, sizeof(Complex<Real>)*pnXY);

		term1(0, 0);
		for_i(pnXY,
			term1[_IM] = 2 * M_PI*(gFreq[_X][i] * mesh[0] + gFreq[_Y][i] * mesh[1] + gFreq[_Z][i] * mesh[2]);
		rearAS[i] = angularSpectrum[i] * exp(term1) * dfx * dfy;
		);
		refASInner_flat();			// refAS main function including texture mapping 

		if (randPhase == true) {
			phase(0, 0);
			for_i(pnXY,
				randVal = rand(0.0, 1.0, i);
			phase[_IM] = 2 * M_PI*randVal;

			convol[i] = shadingFactor*exp(phase) - rearAS[i];
			);
			conv_fft2_scale(refAS, convol, refAS, context_.pixel_number);
		}
		else {
			conv_fft2_scale(rearAS, refAS, convol, context_.pixel_number);

			for_i(pnXY,
				refAS[i] = refAS[i] * shadingFactor - convol[i];
			);
		}
	}
	else {

		refASInner_flat();			// refAS main function including texture mapping 

		if (randPhase == true) {
			phase(0, 0);
			for_i(pnXY,
				randVal = rand(0.0, 1.0, i);
			phase[_IM] = 2 * M_PI*randVal;
			phaseTerm[i] = shadingFactor*exp(phase);
			);
			conv_fft2_scale(refAS, phaseTerm, refAS, context_.pixel_number);
		}
		else {
			for_i(pnXY,
				refAS[i] *= shadingFactor;
			);
		}
	}
	return true;
}


void ophTri::refASInner_flat() {

	for (int i = 0; i < pnXY; i++) {
		//freqTermX[i] += 0.00000001;
		//freqTermY[i] += 0.00000001;
		if (textureMapping == true) {
			refAS[i] = 0;
			for (int idxFy = -texture.dim[_Y] / 2; idxFy < texture.dim[_Y] / 2; idxFy++) {
				for (int idxFx = -texture.dim[_X] / 2; idxFx < texture.dim[_X] / 2; idxFy++) {
					textFreqX = idxFx*texture.freq;
					textFreqY = idxFy*texture.freq;

					tempFreqTermX[i] = freqTermX[i] - textFreqX;
					tempFreqTermY[i] = freqTermY[i] - textFreqY;

					if (tempFreqTermX[i] == -tempFreqTermY[i] && tempFreqTermY[i] != 0.0) {
						refTerm1[_IM] = 2.0 * M_PI*tempFreqTermY[i];
						refTerm2[_IM] = 1.0;
						refTemp = ((Complex<Real>)1.0 - exp(refTerm1)) / (4.0 * M_PI*M_PI*tempFreqTermY[i] * tempFreqTermY[i]) + refTerm2 / (2.0 * M_PI*tempFreqTermY[i]);
						refAS[i] = refAS[i] + textFFT[idxFx + texture.dim[_X] / 2 + (idxFy + texture.dim[_Y] / 2)*texture.dim[_X]] * refTemp;

					}
					else if (tempFreqTermX[i] == tempFreqTermY[i] && tempFreqTermX[i] == 0.0) {
						refTemp = (Real)(1.0 / 2.0);
						refAS[i] = refAS[i] + textFFT[idxFx + texture.dim[_X] / 2 + (idxFy + texture.dim[_Y] / 2)*texture.dim[_X]] * refTemp;
					}
					else if (tempFreqTermX[i] != 0.0 && tempFreqTermY[i] == 0.0) {
						refTerm1[_IM] = -2.0 * M_PI*tempFreqTermX[i];
						refTerm2[_IM] = 1.0;
						refTemp = (exp(refTerm1) - (Complex<Real>)1.0) / (2.0 * M_PI*tempFreqTermX[i] * 2.0 * M_PI*tempFreqTermX[i]) + (refTerm2 * exp(refTerm1)) / (2.0 * M_PI*tempFreqTermX[i]);
						refAS[i] = refAS[i] + textFFT[idxFx + texture.dim[_X] / 2 + (idxFy + texture.dim[_Y] / 2)*texture.dim[_X]] * refTemp;
					}
					else if (tempFreqTermX[i] == 0.0 && tempFreqTermY[i] != 0.0) {
						refTerm1[_IM] = 2.0 * M_PI*tempFreqTermY[i];
						refTerm2[_IM] = 1.0;
						refTemp = ((Complex<Real>)1.0 - exp(refTerm1)) / (4.0 * M_PI*M_PI*tempFreqTermY[i] * tempFreqTermY[i]) - refTerm2 / (2.0 * M_PI*tempFreqTermY[i]);
						refAS[i] = refAS[i] + textFFT[idxFx + texture.dim[_X] / 2 + (idxFy + texture.dim[_Y] / 2)*texture.dim[_X]] * refTemp;
					}
					else {
						refTerm1[_IM] = -2.0 * M_PI*tempFreqTermX[i];
						refTerm2[_IM] = -2.0 * M_PI*(tempFreqTermX[i] + tempFreqTermY[i]);
						refTemp = (exp(refTerm1) - (Complex<Real>)1.0) / (4.0 * M_PI*M_PI*tempFreqTermX[i] * tempFreqTermY[i]) + ((Complex<Real>)1.0 - exp(refTerm2)) / (4.0 * M_PI*M_PI*tempFreqTermY[i] * (tempFreqTermX[i] + tempFreqTermY[i]));
						refAS[i] = refAS[i] + textFFT[idxFx + texture.dim[_X] / 2 + (idxFy + texture.dim[_Y] / 2)*texture.dim[_X]] * refTemp;
					}

				}
			}
		}
		else {
			if (freqTermX[i] == -freqTermY[i] && freqTermY[i] != 0.0) {
				refTerm1[_IM] = 2.0 * M_PI*freqTermY[i];
				refTerm2[_IM] = 1.0;
				refAS[i] = ((Complex<Real>)1.0 - exp(refTerm1)) / (4.0 * M_PI*M_PI*freqTermY[i] * freqTermY[i]) + refTerm2 / (2.0 * M_PI*freqTermY[i]);
			}
			else if (freqTermX[i] == freqTermY[i] && freqTermX[i] == 0.0) {
				refAS[i] = (Real)(1.0 / 2.0);
			}
			else if (freqTermX[i] != 0 && freqTermY[i] == 0.0) {
				refTerm1[_IM] = -2.0 * M_PI*freqTermX[i];
				refTerm2[_IM] = 1.0;
				refAS[i] = (exp(refTerm1) - (Complex<Real>)1.0) / (2.0 * M_PI*freqTermX[i] * 2.0 * M_PI*freqTermX[i]) + (refTerm2 * exp(refTerm1)) / (2.0 * M_PI*freqTermX[i]);
			}
			else if (freqTermX[i] == 0 && freqTermY[i] != 0.0) {
				refTerm1[_IM] = 2.0 * M_PI*freqTermY[i];
				refTerm2[_IM] = 1.0;
				refAS[i] = ((Complex<Real>)1.0 - exp(refTerm1)) / (4.0 * M_PI*M_PI*freqTermY[i] * freqTermY[i]) - refTerm2 / (2.0 * M_PI*freqTermY[i]);
			}
			else {
				refTerm1[_IM] = -2.0 * M_PI*freqTermX[i];
				refTerm2[_IM] = -2.0 * M_PI*(freqTermX[i] + freqTermY[i]);
				refAS[i] = (exp(refTerm1) - (Complex<Real>)1.0) / (4.0 * M_PI*M_PI*freqTermX[i] * freqTermY[i]) + ((Complex<Real>)1.0 - exp(refTerm2)) / (4.0 * M_PI*M_PI*freqTermY[i] * (freqTermX[i] + freqTermY[i]));
			}
		}
	}
	//for (int i = 0; i < 10; i++) {
	//	cout << refAS[i + 45000] << endl;
	//}
}

void ophTri::genRandPhase(ivec2 pixel_number) {

	phase(0, 0);
	for_i(pnXY,
		randVal = rand((Real)0, (Real)1, i);
	phase[_IM] = 2 * M_PI*randVal;
	phaseTerm[i] = exp(phase);
	);
}


bool ophTri::refAS_Continuous(uint n)
{
	av = (0, 0, 0);
	av[0] = nv[3 * n + 0][0] * illumination[0] + nv[3 * n + 0][1] * illumination[1] + nv[3 * n + 0][2] * illumination[2] + 0.1;
	av[2] = nv[3 * n + 1][0] * illumination[0] + nv[3 * n + 1][1] * illumination[1] + nv[3 * n + 1][2] * illumination[2] + 0.1;
	av[1] = nv[3 * n + 2][0] * illumination[0] + nv[3 * n + 2][1] * illumination[1] + nv[3 * n + 2][2] * illumination[2] + 0.1;

	D1(0, 0);
	D2(0, 0);
	D3(0, 0);

	refTerm1(0, 0);
	refTerm2(0, 0);
	refTerm3(0, 0);


	for (i = 0; i < pnXY; i++) {
		if (freqTermX[i] == 0.0 && freqTermY[i] == 0.0) {
			D1(1.0 / 3.0, 0);
			D2(1.0 / 5.0, 0);
			D3(1.0 / 2.0, 0);
		}
		else if (freqTermX[i] == 0.0 && freqTermY[i] != 0.0) {
			refTerm1[_IM] = -2 * M_PI*freqTermY[i];
			refTerm2[_IM] = 1;

			D1 = (refTerm1 - 1.0)*refTerm1.exp() / (8.0 * M_PI*M_PI*M_PI*freqTermY[i] * freqTermY[i] * freqTermY[i])
				- refTerm1 / (4.0 * M_PI*M_PI*M_PI*freqTermY[i] * freqTermY[i] * freqTermY[i]);
			D2 = -(M_PI*freqTermY[i] + refTerm2) / (4.0 * M_PI*M_PI*M_PI*freqTermY[i] * freqTermY[i] * freqTermY[i])*exp(refTerm1)
				+ refTerm1 / (8.0 * M_PI*M_PI*M_PI*freqTermY[i] * freqTermY[i] * freqTermY[i]);
			D3 = exp(refTerm1) / (2.0 * M_PI*freqTermY[i]) + (1.0 - refTerm2) / (2.0 * M_PI*freqTermY[i]);
		}
		else if (freqTermX[i] != 0.0 && freqTermY[i] == 0.0) {
			refTerm1[_IM] = 4.0 * M_PI*M_PI*freqTermX[i] * freqTermX[i];
			refTerm2[_IM] = 1.0;
			refTerm3[_IM] = 2.0 * M_PI*freqTermX[i];

			D1 = (refTerm1 + 4.0 * M_PI*freqTermX[i] - 2.0 * refTerm2) / (8.0 * M_PI*M_PI*M_PI*freqTermY[i] * freqTermY[i] * freqTermY[i])*exp(-refTerm3)
				+ refTerm2 / (4.0 * M_PI*M_PI*M_PI*freqTermX[i] * freqTermX[i] * freqTermX[i]);
			D2 = 1.0 / 2.0 * D1;
			D3 = ((refTerm3 + 1.0)*exp(-refTerm3) - 1.0) / (4.0 * M_PI*M_PI*freqTermX[i] * freqTermX[i]);
		}
		else if (freqTermX[i] == -freqTermY[i]) {
			refTerm1[_IM] = 1.0;
			refTerm2[_IM] = 2.0 * M_PI*freqTermX[i];
			refTerm3[_IM] = 2.0 * M_PI*M_PI*freqTermX[i] * freqTermX[i];

			D1 = (-2.0 * M_PI*freqTermX[i] + refTerm1) / (8.0 * M_PI*M_PI*M_PI*freqTermX[i] * freqTermX[i] * freqTermX[i])*exp(-refTerm2)
				- (refTerm3 + refTerm1) / (8.0 * M_PI*M_PI*M_PI*freqTermX[i] * freqTermX[i] * freqTermX[i]);
			D2 = (-refTerm1) / (8.0 * M_PI*M_PI*M_PI*freqTermX[i] * freqTermX[i] * freqTermX[i])*exp(-refTerm2)
				+ (-refTerm3 + refTerm1 + 2.0 * M_PI*freqTermX[i]) / (8.0 * M_PI*M_PI*M_PI*freqTermX[i] * freqTermX[i] * freqTermX[i]);
			D3 = (-refTerm1) / (4.0 * M_PI*M_PI*freqTermX[i] * freqTermX[i])*exp(-refTerm2)
				+ (-refTerm2 + 1.0) / (4.0 * M_PI*M_PI*freqTermX[i] * freqTermX[i]);
		}
		else {
			refTerm1[_IM] = -2.0 * M_PI*(freqTermX[i] + freqTermY[i]);
			refTerm2[_IM] = 1.0;
			refTerm3[_IM] = -2.0 * M_PI*freqTermX[i];

			D1 = exp(refTerm1)*(refTerm2 - 2.0 * M_PI*(freqTermX[i] + freqTermY[i])) / (8 * M_PI*M_PI*M_PI*freqTermY[i] * (freqTermX[i] + freqTermY[i])*(freqTermX[i] + freqTermY[i]))
				+ exp(refTerm3)*(2.0 * M_PI*freqTermX[i] - refTerm2) / (8.0 * M_PI*M_PI*M_PI*freqTermX[i] * freqTermX[i] * freqTermY[i])
				+ ((2.0 * freqTermX[i] + freqTermY[i])*refTerm2) / (8.0 * M_PI*M_PI*M_PI*freqTermX[i] * freqTermX[i] * (freqTermX[i] + freqTermY[i])*(freqTermX[i] + freqTermY[i]));
			D2 = exp(refTerm1)*(refTerm2*(freqTermX[i] + 2.0 * freqTermY[i]) - 2.0 * M_PI*freqTermY[i] * (freqTermX[i] + freqTermY[i])) / (8.0 * M_PI*M_PI*M_PI*freqTermY[i] * freqTermY[i] * (freqTermX[i] + freqTermY[i])*(freqTermX[i] + freqTermY[i]))
				+ exp(refTerm3)*(-refTerm2) / (8.0 * M_PI*M_PI*M_PI*freqTermX[i] * freqTermY[i] * freqTermY[i])
				+ refTerm2 / (8.0 * M_PI*M_PI*M_PI*freqTermX[i] * (freqTermX[i] + freqTermY[i])* (freqTermX[i] + freqTermY[i]));
			D3 = -exp(refTerm1) / (4.0 * M_PI*M_PI*freqTermY[i] * (freqTermX[i] + freqTermY[i]))
				+ exp(refTerm3) / (4.0 * M_PI*M_PI*freqTermX[i] * freqTermY[i])
				- 1.0 / (4.0 * M_PI*M_PI*freqTermX[i] * (freqTermX[i] + freqTermY[i]));
		}
		refAS[i] = (av[1] - av[0])*D1 + (av[2] - av[1])*D2 + av[0] * D3;
	}
	if (randPhase == true) {
		phase(0, 0);
		for_i(pnXY,
			randVal = rand(0.0, 1.0, i);
		phase[_IM] = 2.0 * M_PI*randVal;
		phaseTerm[i] = exp(phase);
		);
		conv_fft2_scale(refAS, phaseTerm, convol, context_.pixel_number);
	}

	return true;
}

bool ophTri::refToGlobal()
{

	term1(0, 0);
	term2(0, 0);
	det = geom.loRot[0] * geom.loRot[3] - geom.loRot[1] * geom.loRot[2];
	if (det == 0)
		return -1;
	if (det < 0)
		det = -det;

	for (i = 0; i < pnXY; i++) {
		if (gFreq[_Z][i] == 0)
			term2 = 0;
		else {
			term1[_IM] = 2 * M_PI*(lFreq[_X][i] * geom.glShift[_X] + lFreq[_Y][i] * geom.glShift[_Y] + lFreq[_Z][i] * geom.glShift[_Z]);
			term2 = refAS[i] / det * lFreq[_Z][i] / gFreq[_Z][i] * exp(term1);// *phaseTerm[i];
		}
		if (abs(term2) > MIN_DOUBLE) {}
		else { term2 = 0; }
		angularSpectrum[i] += term2;
	}

	return true;
}


void ophTri::reconTest(const char* fname) {
	Complex<Real>* recon = new Complex<Real>[pnXY];
	fresnelPropagation((*complex_H), recon, context_.shift[_Z], 0);
	encoding(ENCODE_AMPLITUDE, recon);
	normalize();
	save(fname, 8, nullptr, m_vecEncodeSize[_X], m_vecEncodeSize[_Y]);
}

// correct the output scale of the  ophGen::conv_fft2 
void ophTri::conv_fft2_scale(Complex<Real>* src1, Complex<Real>* src2, Complex<Real>* dst, ivec2 size) {

	const double double_nXY = size[_X] * size[_Y];

	src1FT = new Complex<Real>[size[_X] * size[_Y]];
	src2FT = new Complex<Real>[size[_X] * size[_Y]];
	dstFT = new Complex<Real>[size[_X] * size[_Y]];


	fftwShift(src1, src1FT, size[_X], size[_Y], OPH_FORWARD, (bool)OPH_ESTIMATE);

	fftwShift(src2, src2FT, size[_X], size[_Y], OPH_FORWARD, (bool)OPH_ESTIMATE);


	for (int i = 0; i < size[_X] * size[_Y]; i++)
		dstFT[i] = src1FT[i] * src2FT[i] * double_nXY * double_nXY;

	fftwShift(dstFT, dst, size[_X], size[_Y], OPH_BACKWARD, (bool)OPH_ESTIMATE);

	delete[] src1FT, src2FT, dstFT;
}