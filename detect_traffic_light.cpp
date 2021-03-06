#include <opencv2/opencv.hpp>

#define min(d, s) ((d < s) ? (d) : (s))
#define max(d, s) ((d > s) ? (d) : (s))

using namespace cv;
using namespace std;

const char* path_image = "CamVidLights/CamVidLights%02d.png"; //format
const char* path_circle = "files/circle.png";
const char* path_model = "files/model.png";
const char* path_bp_red = "files/red-bp.png"; //backprojection, red
const char* path_bp_amber = "files/amber-bp.png"; //backprojection, amber
const char* path_bp_green = "files/green-bp.png"; //backprojection, green

#define GT_SIZE 14 //no of ground truth files
#define GT_MAX_LIGHTS 4 //no of max lights per image

//ground truth, considering full light
int gt_full_light[GT_SIZE][GT_MAX_LIGHTS][4] = {
    {{319, 202, 346, 279}, {692, 264, 711, 322}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{217, 103, 261, 230}, {794, 212, 820, 294}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{347, 210, 373, 287}, {701, 259, 720, 318}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{271,  65, 339, 191}, {640, 260, 662, 301}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{261,  61, 333, 195}, {644, 269, 666, 313}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{238,  42, 319, 190}, {650, 279, 672, 323}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{307, 231, 328, 297}, {747, 266, 764, 321}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{280, 216, 305, 296}, {795, 253, 816, 316}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{359, 246, 380, 305}, {630, 279, 646, 327}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{233, 122, 299, 239}, {681, 271, 714, 315}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{331, 260, 349, 312}, {663, 280, 676, 322}, {0, 0, 0, 0}, {0, 0, 0, 0}}, 
    {{373, 219, 394, 279}, {715, 242, 732, 299}, {423, 316, 429, 329}, {516, 312, 521, 328}},
    {{272, 211, 299, 261}, {604, 233, 620, 279}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{279, 188, 315, 253}, {719, 225, 740, 286}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

//ground truth, considering central rectangle
int gt_main_light[GT_SIZE][GT_MAX_LIGHTS][4] = {
    {{319, 202, 346, 279}, {692, 264, 711, 322}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{217, 103, 261, 230}, {794, 212, 820, 294}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{347, 210, 373, 287}, {701, 259, 720, 318}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{271,  65, 309, 189}, {640, 260, 652, 301}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{261,  61, 302, 193}, {644, 269, 657, 312}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{238,  42, 284, 187}, {650, 279, 663, 323}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{307, 231, 328, 297}, {747, 266, 764, 321}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{280, 216, 305, 296}, {795, 253, 816, 316}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{359, 246, 380, 305}, {630, 279, 646, 327}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{260, 122, 299, 239}, {691, 271, 705, 315}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{331, 260, 349, 312}, {663, 280, 676, 322}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{373, 219, 394, 279}, {715, 242, 732, 299}, {423, 316, 429, 329}, {516, 312, 521, 328}},
    {{283, 211, 299, 261}, {604, 233, 620, 279}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{294, 188, 315, 253}, {719, 225, 740, 286}, {0, 0, 0, 0}, {0, 0, 0, 0}}
};

char gt_light_state[GT_SIZE][GT_MAX_LIGHTS] = {
    {'G', 'G', '\0', '\0'},
    {'G', 'G', '\0', '\0'},
    {'G', 'G', '\0', '\0'},
    {'R', 'R', '\0', '\0'},
    {'R'+'A', 'R'+'A', '\0', '\0'},
    {'G', 'G', '\0', '\0'},
    {'A', 'A', '\0', '\0'},
    {'A', 'A', '\0', '\0'},
    {'G', 'G', '\0', '\0'},
    {'G', 'G', '\0', '\0'},
    {'G', 'G', '\0', '\0'},
    {'G', 'G',  'R',  'R'},
    {'R', 'R', '\0', '\0'},
    {'R', 'R', '\0', '\0'}
};

const int max_low_threshold = 100;
int ratio1 = 4;
int kernel_size = 3;
int low_threshold = 50;
int test_thresh = 5;
RNG rng(12345);

void getBackProjection(string filename, Mat &target, Mat &thresh) {

    Mat roi = imread(filename); //region of interest
    Mat roi_hsv, target_hsv;
    cvtColor(roi, roi_hsv, COLOR_BGR2HSV, 0);
    cvtColor(target, target_hsv, COLOR_BGR2HSV, 0);

    MatND roi_hist;
    int channels[] = {0, 1};
    int hbins = 180, sbins = 256;
    int histSize[] = {hbins, sbins};
    float hranges[] = { 0, 180 };
    float sranges[] = { 0, 256 };
    const float* ranges[] = { hranges, sranges };
    calcHist(&roi_hsv, 1, channels, Mat(), roi_hist, 2, histSize, ranges, true, false);
    normalize(roi_hist, roi_hist, 0, 255, NORM_MINMAX);
    calcBackProject(&target_hsv, 1, channels, roi_hist, thresh, ranges, 1, true);

    Mat disc = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(-1,-1));
    
    filter2D(thresh, thresh, -1, disc, Point(-1, -1), 0, BORDER_DEFAULT);
    threshold(thresh, thresh, 100, 255, THRESH_BINARY);
}

/*
 * adapted from the book: ISBN 978-1-118-84845-6
 * optimized for single channel images, and made function more readable by me.
 */
void chamferMatching(Mat &chamfer_image, Mat &model, Mat &matching_image) {
    // Extract the model points (as they are sparse).
    vector<Point> model_points;
    for (int y = 0; y < model.rows; y++)
        for (int x = 0; x < model.cols; x++)
            if (model.at<uchar>(y, x) > 0)
                model_points.push_back(Point(x, y));

    // Try the model in every possible position
    matching_image = Mat::zeros(chamfer_image.rows - model.rows + 1, chamfer_image.cols - model.cols + 1, CV_32FC1);
    for (int y = 0; y < matching_image.rows; y++)
        for (int x = 0; x < matching_image.cols; x++)
            for (int i = 0; i < model_points.size(); i++) 
                matching_image.at<float>(y, x) += chamfer_image.at<float>(y + model_points[i].y, x + model_points[i].x);
}

void getBlackVsWhitePixels(Mat &bin_image, int &white, int &black) {
    white = black = 0;
    for (int i = 0; i < bin_image.rows; i++)
        for (int j = 0; j < bin_image.cols; j++)
            if ((bin_image.at<uchar>(i, j)) == 255) white++;
            else black++;
}

/*
 * @returns true if new image is completely empty
 */
bool cleanBinaryImageOutsideRect(Mat &image, Rect &rect, int id) {

    Mat mask = Mat::zeros(image.size(), CV_8UC1);
    rectangle(mask, rect, Scalar(255), CV_FILLED);

    bitwise_and(image, mask, image);

    return countNonZero(image) == 0;
}

/* if contour doesn't has a parent (this happens when outer frame of traffic
 * light is not a closed loop in edge image), try to find a rectangle around
 * the @light.
 */
Rect findParent(Mat edge_image, Mat &model_orig, pair<Rect, char> &light_color, int id) {

    Mat chamfer_image, model, matching_image;
    const float h_ratio = 0.20f, w_ratio = 0.40f; // light to frame (inc borders)

    Rect &light = light_color.first;
    edge_image = edge_image.clone();

    int model_width = 1.0f * light.width / w_ratio;
    int model_height = 1.0f * model_orig.rows / model_orig.cols * model_width;

    resize(model_orig, model, Size(model_width, model_height), 0, 0, INTER_CUBIC);
    Canny(model, model, 50, 100, 3);

    float err = 0.35f;
    int color = light_color.second == 'G' ? 1 : light_color.second == 'R' ? -1 : 0;
    int x1 = max(0, light.x + light.width / 2 - err * model_width - model_width / 2);
    int y1 = max(0, light.y + light.height / 2 - err * model_height - model_height / 2 - color * 1.0f * model_width / 3);
    Rect range(x1, y1, min((1 + err) * model_width, edge_image.cols - 1 - x1), min((1 + err) * model_height, edge_image.rows - 1 - y1));
    bool empty_image = cleanBinaryImageOutsideRect(edge_image, range, id);

    if (empty_image)
        return Rect(0, 0, 0, 0);

    //based on light color, crop the image
    threshold(edge_image, chamfer_image, 127, 255, THRESH_BINARY_INV);
    distanceTransform(chamfer_image, chamfer_image, CV_DIST_L2, 3);
    normalize(chamfer_image, chamfer_image, 0, 1.0, NORM_MINMAX);
    chamferMatching(chamfer_image, model, matching_image);
    Mat test_matching_image;
    normalize(matching_image, test_matching_image, 0, 1.0, NORM_MINMAX);

    float best_score = FLT_MAX;
    Point best_point;
    for (int i = 0; i < matching_image.rows; i++)
        for (int j = 0; j < matching_image.cols; j++) {
            float score = matching_image.at<float>(i, j);
            if (score < best_score) {
                best_score = score;
                best_point = Point(j, i);
            }
        }

    return Rect(best_point.x, best_point.y, model_width, model_height);
}

Rect getAverageRect(Rect rect1, Rect rect2) {
    Rect rect;
    rect.x = (rect1.x + rect2.x) / 2;
    rect.y = (rect1.y + rect2.y) / 2;
    rect.width = (rect1.x + rect1.width + rect2.x + rect2.width) / 2 - rect.x;
    rect.height = (rect1.y + rect1.height + rect2.y + rect2.height) / 2 - rect.y;
    return rect;
}

/*
 * if the two rectangles overlap, average them
 */
void mergeTwoParents(Rect &rect1, Rect &rect2) {
    Rect intersection = rect1 & rect2;
    if (intersection.area() == 0)
        return;

    rect1 = rect2 = getAverageRect(rect1, rect2);
}

//find ellipses around bitmap
void findLights(Mat &source, Mat &edge_image, Mat &model, vector< pair< pair<Rect, char>, pair<Rect, bool> > > &lights) {

    Mat &drawing = source;
    vector< vector<Point> > circle_contours;
    Mat circle = imread(path_circle, CV_LOAD_IMAGE_COLOR);
    cvtColor(circle, circle, CV_BGR2GRAY);
    Canny(circle, circle, 50, 100, 3);
    vector<Vec4i> circle_hierarchy;

    findContours(circle, circle_contours, circle_hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
    if (circle_contours.size() < 1)
        cout << "Error: No contour found in cicle image" << endl;

    vector<Point> circle_contour = circle_contours[0];
    Mat b_proj_lights_map_red, b_proj_lights_map_amber, b_proj_lights_map_green;
    getBackProjection(path_bp_red, source, b_proj_lights_map_red);
    getBackProjection(path_bp_amber, source, b_proj_lights_map_amber);
    getBackProjection(path_bp_green, source, b_proj_lights_map_green);

    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(edge_image, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

    for (int i = 0; i< contours.size(); i++) {

        vector<Point> contour = contours[i];
        const float epsilon = 0.01f;
        vector<Point> approx;
        approxPolyDP(contour, approx, epsilon * arcLength(contour, true), true);

        if (approx.size() > test_thresh && hierarchy[i][2] > 0 &&
            matchShapes(contour, circle_contour, CV_CONTOURS_MATCH_I1, 0) < 0.1) {

            int white, black;
            Rect light = boundingRect(contour);
            Mat bin_image = b_proj_lights_map_green(light);
            getBlackVsWhitePixels(bin_image, white, black);

            pair<Rect, bool> frame;
            if (1.0f * white / (black + white) > 0.4f) {
                
                //average the light, with min max coordinates
                light = getAverageRect(Rect(light.x, light.y, min(light.width, light.height), min(light.width, light.height)), Rect(light.x, light.y, max(light.width, light.height), max(light.width, light.height)));
                pair<Rect, char> light_color = make_pair(light, 'G');

                int parent = hierarchy[i][3];
                if (parent != -1)
                    frame = make_pair(boundingRect(contours[parent]), true);
                else
                    frame = make_pair(findParent(edge_image, model, light_color, i), false);

                lights.push_back(make_pair(light_color, frame));
                continue;
            }

            bin_image = b_proj_lights_map_amber(light);
            getBlackVsWhitePixels(bin_image, white, black); 
            if (1.0f * white / (black + white) > 0.3f) {

                light = getAverageRect(Rect(light.x, light.y, min(light.width, light.height), min(light.width, light.height)), Rect(light.x, light.y, max(light.width, light.height), max(light.width, light.height)));
                pair<Rect, char> light_color = make_pair(light, 'A');

                int parent = hierarchy[i][3];
                if (parent != -1)
                    frame = make_pair(boundingRect(contours[parent]), true);
                else
                    frame = make_pair(findParent(edge_image, model, light_color, i), false);

                lights.push_back(make_pair(light_color, frame));
                continue;
            }

            bin_image = b_proj_lights_map_red(light);
            getBlackVsWhitePixels(bin_image, white, black);
            if (1.0f * white / (black + white) > 0.6f) {

                light = getAverageRect(Rect(light.x, light.y, min(light.width, light.height), min(light.width, light.height)), Rect(light.x, light.y, max(light.width, light.height), max(light.width, light.height)));
                pair<Rect, char> light_color = make_pair(light, 'R');

                int parent = hierarchy[i][3];
                if (parent != -1)
                    frame = make_pair(boundingRect(contours[parent]), true);
                else
                    frame = make_pair(findParent(edge_image, model, light_color, i), false);

                lights.push_back(make_pair(light_color, frame));
            }
        }
    }

    //remove inner circle of same color
    for (int i = 0; i < lights.size(); i++)
        for (int j = i + 1; j < lights.size(); j++) {
            if ((lights[i].first.first & lights[j].first.first).area() > 0) {
                if (lights[i].first.first.area() >= lights[j].first.first.area())
                    lights.erase(lights.begin() + j);
                else {
                    lights.erase(lights.begin() + i);
                    j--;
                }
            }
        }

    //merge two parents of different lights
    for (int i = 0; i < lights.size(); i++)
        for (int j = i + 1; j < lights.size(); j++)
            mergeTwoParents(lights[i].second.first, lights[j].second.first);
}

int main(int argc, char** argv) {

    char buf[1024];
    int file_number = stoi(argv[1]);
    sprintf(buf, path_image, file_number);
    
    Mat src_gray, edge_image;
    Mat source = imread(buf, CV_LOAD_IMAGE_COLOR);
    Mat model = imread(path_model);

    cvtColor(source, src_gray, CV_BGR2GRAY);
    cvtColor(model, model, CV_BGR2GRAY);
    threshold(model, model, 0, 255, THRESH_BINARY);
    
    Mat disc = getStructuringElement(MORPH_ELLIPSE, Size(2, 2), Point(-1, -1));
    
    GaussianBlur(src_gray, src_gray, Size(0, 0), 1.5);
    addWeighted(src_gray, 3.5, src_gray, -1.5, 0, src_gray);
    
    Canny(src_gray, edge_image, low_threshold, low_threshold * ratio1, kernel_size);
    threshold(edge_image, edge_image, 0, 255, THRESH_BINARY);
    
    vector< pair< pair<Rect, char>, pair<Rect, bool> > > lights;
    findLights(source, edge_image, model, lights);

    bool gt_light_drawn[GT_SIZE][GT_MAX_LIGHTS] = {false};
    for (int i = 0; i < lights.size(); i++) {
        pair<Rect, char> &light_color = lights[i].first;
        pair<Rect, bool> &frame_full = lights[i].second;
        cout << "Recoginized Light: " << i << endl;
        cout << "    Frame: (x1, y1), (x2, y2): (" << frame_full.first.x << ", " << frame_full.first.y << "), (" << frame_full.first.x + frame_full.first.width << ", " << frame_full.first.y + frame_full.first.height << ")" << endl;
        cout << "    Frame: Full?: " << (frame_full.second ? "true" : "false") << endl;
        cout << "    State: " << light_color.second << endl;

        Scalar color;
        if (light_color.second == 'R')
            color = Scalar(54, 67, 244);
        else if (light_color.second == 'A')
            color = Scalar(7, 193, 255);
        else if (light_color.second == 'G')
            color = Scalar(74, 195, 139);

        Rect &light = light_color.first;
        RotatedRect light_rr = RotatedRect(Point(light.x + light.width / 2, light.y + light.height / 2), light.size(), 0);
        ellipse(source, light_rr, color, CV_FILLED, LINE_AA);

        //find rect from ground truth and draw
        int (*gt_light)[GT_MAX_LIGHTS][4] = frame_full.second ? &gt_full_light[0] : &gt_main_light[0];
        for (int j = 0; j < GT_MAX_LIGHTS; j++) {
            int x = gt_light[file_number - 1][j][0];
            int y = gt_light[file_number - 1][j][1];
            int width = gt_light[file_number - 1][j][2] - x;
            int height = gt_light[file_number - 1][j][3] - y;
            Rect gt_frame = Rect(x, y, width, height);
            Rect intersection = gt_frame & frame_full.first;
            if (width != 0 && height != 0 && intersection.area() > 0) {

                rectangle(source, gt_frame, Scalar(0, 255, 0));

                char gt_state = gt_light_state[file_number - 1][j];
                cout << "    State: Correct?: " << (light_color.second == gt_state ? "true" : "false") << endl;
                float dsc = 2.0f * intersection.area() / (frame_full.first.area() + gt_frame.area());
                cout << "    DSC: " << dsc << endl;

                gt_light_drawn[file_number - 1][j] = true;
            }
        }

        rectangle(source, frame_full.first, Scalar(0, 0, 255));
    }

    //print gt which aren't detected
    for (int j = 0; j < GT_MAX_LIGHTS; j++)
        if (!gt_light_drawn[file_number - 1][j]) {
            int x = gt_main_light[file_number - 1][j][0];
            int y = gt_main_light[file_number - 1][j][1];
            int width = gt_main_light[file_number - 1][j][2] - x;
            int height = gt_main_light[file_number - 1][j][3] - y;
            Rect rect = Rect(x, y, width, height);
            if (width != 0 && height != 0) {
                cout << "Groundtruth Light Not detected:" << endl;
                cout << "    Frame: " << rect << endl;
                cout << "    State: " << gt_light_state[file_number - 1][j] << endl;
                cout << "    DSC: " << 0 << endl;
                
                rectangle(source, rect, Scalar(0, 255, 0));
            }
        }

    imshow("Result Image", source);

    waitKey(0);
    return 0;
}
