#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <opencv2/opencv.hpp>

struct BaseImage {
    virtual std::string convertToString() const = 0;
};

struct GrayScaleImage final: public BaseImage {
    const std::vector<int8_t> data;
    const int width;
    const int height;

    GrayScaleImage(const std::vector<int8_t>& data, int width, int height) noexcept
        : data(data), width(width), height(height)
    {
    }

    void showWithOpenCV() const {
        std::vector<uint8_t> udata(data.begin(), data.end());
        cv::Mat img(height, width, CV_8UC1, udata.data());
        cv::imshow("Gray Image", img);
        cv::waitKey(0);
    }

    std::string convertToString() const override
    {
        std::ostringstream os;
        os << +(*data.begin());
        for_each(data.begin() + 1, data.end(), [&os](int8_t p) {
            os << ", " << +p;
        });
        return "[" + os.str() + "]";
    }

};

class ImageConverter final {
public:
    ImageConverter& readImage(const std::string& image) noexcept {
        std::cout << "read image:" << image << std::endl;
        FILE* fp = fopen(image.c_str(), "rb");
        if (!fp) {
            std::cerr << "ファイルを開けません: " << image << std::endl;
            std::exit(1);
        }
        fseek(fp, 18, SEEK_SET);
        int w = 0, h = 0;
        fread(&w, 4, 1, fp);
        fread(&h, 4, 1, fp);
        width_ = w;
        height_ = h;
        fseek(fp, 54, SEEK_SET);
        imageData_.resize(width_ * height_);
        int rowSize = ((width_ * 3 + 3) / 4) * 4;
        std::vector<unsigned char> rowBuf(rowSize);
        for (int y = 0; y < height_; ++y) {
            fread(rowBuf.data(), 1, rowSize, fp);
            for (int x = 0; x < width_; ++x) {
                int idx = y * width_ + x;
                unsigned char b = rowBuf[x * 3 + 0];
                unsigned char g = rowBuf[x * 3 + 1];
                unsigned char r = rowBuf[x * 3 + 2];
                imageData_[idx] = std::array<uint8_t, 3>{r, g, b};
            }
        }
        fclose(fp);
        return *this;
    }

    ImageConverter& peekPrintImageData() noexcept
    {
        std::cout << "print image data" << std::endl;
        return *this;
    }

    GrayScaleImage convertToGrayScale() const noexcept
    {
        constexpr double R_WEIGHT = 0.299;
        constexpr double G_WEIGHT = 0.587;
        constexpr double B_WEIGHT = 0.114;
        
        std::cout << "convert image to gray scale" << std::endl;
        std::vector<int8_t> grayData;
        grayData.reserve(imageData_.size());
        
        for (const auto& rgb : imageData_) {
            int gray = static_cast<int>(R_WEIGHT * rgb[0] + G_WEIGHT * rgb[1] + B_WEIGHT * rgb[2]);
            grayData.push_back(static_cast<int8_t>(gray));
        }
        
        return GrayScaleImage(grayData, width_, height_);
    }


private:
    std::vector<std::array<uint8_t, 3>> imageData_;  // 追加: 画像データを保持
    int width_ = 0, height_ = 0;         // 追加: 寸法保持
};

int main(){
    std::string imageFile = "./hoge.bmp";

    ImageConverter converter;
    GrayScaleImage grayScaleImage = converter
        .readImage(imageFile)
        .peekPrintImageData()
        .convertToGrayScale();

    //std::cout << "gray scale vector: " << grayScaleImage.convertToString() << std::endl;
    
    grayScaleImage.showWithOpenCV();

    return 0;
}
