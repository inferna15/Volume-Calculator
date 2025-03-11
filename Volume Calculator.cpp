#include "Volume Calculator.h"

#define SAMPLE_PATH "C:\\Users\\fatil\\OneDrive\\Belgeler\\Dicoms\\beyin"

VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

using namespace std;

int main()
{
	// Dicom görüntüleri oku
	vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	reader->SetDirectoryName(SAMPLE_PATH);
	reader->Update();

	if (reader->GetDataScalarType() == 4) {
		cout << "Bit derinliği: 8-bit (unsigned char)" << endl;
	}
	else if (reader->GetDataScalarType() == 5) {
		cout << "Bit derinliği: 16-bit (unsigned short)" << endl;
	}

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

	// Görselleştir

	// Hacime alınan
	vtkSmartPointer<vtkSmartVolumeMapper> mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	mapper->SetInputConnection(thresholding->GetOutputPort());
	
	vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
	color->AddRGBPoint(0, 0.1, 0.2, 0.4);   // Siyah
	color->AddRGBPoint(255, 1.0, 1.0, 1.0); // Beyaz

	vtkSmartPointer<vtkPiecewiseFunction> opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacity->AddPoint(0, 1.0);   // Tamamen şeffaf
	opacity->AddPoint(255, 0.0); // Tamamen opak

	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(mapper);
	volume->GetProperty()->SetColor(color);
	volume->GetProperty()->SetScalarOpacity(opacity);
	volume->GetProperty()->ShadeOn();
	volume->GetProperty()->SetAmbient(0.4);
	volume->GetProperty()->SetDiffuse(0.6);
	volume->GetProperty()->SetSpecular(0.2);
	volume->GetProperty()->SetSpecularPower(10.0);

	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->AddVolume(volume);
	renderer->SetBackground(0.4, 0.2, 0.1); // Arka plan rengi
	renderer->SetViewport(0.5, 0, 1, 1);

	// Orijinal
	vtkSmartPointer<vtkSmartVolumeMapper> mapper2 = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	mapper2->SetInputConnection(reader->GetOutputPort());

	vtkSmartPointer<vtkColorTransferFunction> color2 = vtkSmartPointer<vtkColorTransferFunction>::New();
	color2->AddRGBPoint(0, 0.1, 0.2, 0.4);   // Siyah
	color2->AddRGBPoint(255, 1.0, 1.0, 1.0); // Beyaz

	vtkSmartPointer<vtkPiecewiseFunction> opacity2 = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacity2->AddPoint(0, 0.0);   // Tamamen şeffaf
	opacity2->AddPoint(255, 1.0); // Tamamen opak

	vtkSmartPointer<vtkVolume> volume2 = vtkSmartPointer<vtkVolume>::New();
	volume2->SetMapper(mapper2);
	volume2->GetProperty()->SetColor(color2);
	volume2->GetProperty()->SetScalarOpacity(opacity2);
	volume2->GetProperty()->ShadeOn();
	volume2->GetProperty()->SetAmbient(0.4);
	volume2->GetProperty()->SetDiffuse(0.6);
	volume2->GetProperty()->SetSpecular(0.2);
	volume2->GetProperty()->SetSpecularPower(10.0);

	vtkSmartPointer<vtkRenderer> renderer2 = vtkSmartPointer<vtkRenderer>::New();
	renderer2->AddVolume(volume2);
	renderer2->SetBackground(0.4, 0.4, 0.2); // Arka plan rengi
	renderer2->SetViewport(0, 0, 0.5, 1);

	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	renderWindow->AddRenderer(renderer2);
	renderWindow->SetSize(800, 600); // Pencere boyutu

	vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(renderWindow);

	renderWindow->Render();
	interactor->Start();

	return EXIT_SUCCESS;
}
