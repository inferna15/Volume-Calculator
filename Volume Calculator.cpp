#include "Volume Calculator.h"

#define SAMPLE_PATH "C:\\Users\\fatil\\OneDrive\\Belgeler\\Dicoms\\beyin"

using namespace std;

int main()
{
	// Dicom görüntüleri oku
	vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	reader->SetDirectoryName(SAMPLE_PATH);
	reader->Update();
	vtkImageData* inputImage = reader->GetOutput();

	// Dicom görüntünün meta verilerini al
	int* dims = inputImage->GetDimensions();
	int width = dims[0];
	int height = dims[1];
	int numSlices = dims[2];
	int N = width * height * numSlices;

	// Otsu hesaplamaya başla
	int maxIntensity = 255;
	std::vector<int> histogram(maxIntensity + 1, 0);

	unsigned char* pixels = static_cast<unsigned char*>(inputImage->GetScalarPointer());

	for (int i = 0; i < N; i++) {
		int value = pixels[i];
		histogram[value]++;
	}

	double varMax = 0;
	double sum = 0;
	double sumB = 0;
	double q1 = 0;
	double q2 = 0;
	double mu1 = 0;
	double mu2 = 0;
	int threshold = 0;

	for (int i = 0; i <= maxIntensity; i++) {
		sum += i * histogram[i];
	}

	for (int t = 0; t <= maxIntensity; t++) {
		q1 += histogram[t];
		if (q1 == 0) continue;

		q2 = N - q1;
		sumB += t * histogram[t];

		mu1 = sumB / q1;
		mu2 = (sum - sumB) / q2;

		double sigmaB = q1 * q2 * (mu1 - mu2) * (mu1 - mu2);

		if (sigmaB > varMax) {
			varMax = sigmaB;
			threshold = t;
		}
	}

	// Otsu sonucunda çıkan değeri uygula
	vtkSmartPointer<vtkImageThreshold> thresholding = vtkSmartPointer<vtkImageThreshold>::New();
	thresholding->SetInputData(inputImage);
	thresholding->ThresholdByLower(threshold);
	thresholding->SetInValue(255);
	thresholding->SetOutValue(0);
	thresholding->Update();

	vtkImageData* otsuImage = thresholding->GetOutput();

	// Hacmi hesapla
	double* spacing = otsuImage->GetSpacing();
	double voxelVolume = spacing[0] * spacing[1] * spacing[2];

	unsigned char* voxels = static_cast<unsigned char*>(otsuImage->GetScalarPointer());
	int nonEmptyVoxelCount = 0;

	for (int i = 0; i < N; i++) {
		if (voxels[i] == 0) {
			nonEmptyVoxelCount++;
		}
	}

	// Sonucu yazdır
	cout << "Toplam voxel sayısı: " << N << endl;
	cout << "Hacim sayılan voxel sayısı: " << nonEmptyVoxelCount << endl;
	cout << "Hacim sayılmayan voxel sayısı: " << N - nonEmptyVoxelCount << endl;
	cout << "Voxel hacmi: " << voxelVolume << " mm3" << endl;
	cout << "Toplam hacim: " << voxelVolume * nonEmptyVoxelCount / 1000 << " cm3" << endl;

	return EXIT_SUCCESS;
}
