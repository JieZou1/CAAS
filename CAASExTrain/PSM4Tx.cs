using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Drawing;

using HtmlAgilityPack;

using Emgu.CV;
using Emgu.CV.Structure;
using Emgu.CV.CvEnum;

namespace CAASExTrain
{
    class PSM4TxSample
    {
        public string imageFile;
        public double isolatorX, isolatorY, isolatorW, isolatorH;
        public double targetX, targetY, targetW, targetH;
        public double apertureX, apertureY, apertureW, apertureH;


        public PSM4TxSample(string image_file)
        {
            imageFile = image_file;
        }

        public void SetTarget(string s_x, string s_y, string s_w, string s_h)
        {
            targetX = double.Parse(s_x);
            targetY = double.Parse(s_y);
            targetW = double.Parse(s_w);
            targetH = double.Parse(s_h);
        }

        public void SetIsolator(string s_x, string s_y, string s_w, string s_h)
        {
            isolatorX = double.Parse(s_x);
            isolatorY = double.Parse(s_y);
            isolatorW = double.Parse(s_w);
            isolatorH = double.Parse(s_h);
        }

        public void SetAperture(string s_x, string s_y, string s_w, string s_h)
        {
            apertureX = double.Parse(s_x);
            apertureY = double.Parse(s_y);
            apertureW = double.Parse(s_w);
            apertureH = double.Parse(s_h);
        }
    };

    class PSM4Tx
    {
        static int TARGET_ORIGINAL_WIDTH = 500;
        static int TARGET_ORIGINAL_HEIGHT = 2500;
        static int TARGET_HOG_WIDTH = 40;
        static int TARGET_HOG_HEIGHT = 200;

        static int ISOLATOR_ORIGINAL_WIDTH = 500;
        static int ISOLATOR_ORIGINAL_HEIGHT = 500;
        static int ISOLATOR_HOG_WIDTH = 80;
        static int ISOLATOR_HOG_HEIGHT = 80;

        static int APERTURE_ORIGINAL_WIDTH = 240;
        static int APERTURE_ORIGINAL_HEIGHT = 2000;
        static int APERTURE_HOG_WIDTH = 24;
        static int APERTURE_HOG_HEIGHT = 200;

        static Size winSizeTarget = new Size(TARGET_HOG_WIDTH, TARGET_HOG_HEIGHT);
        static Size winSizeIsolator = new Size(ISOLATOR_HOG_WIDTH, ISOLATOR_HOG_HEIGHT);
        static Size winSizeAperture = new Size(APERTURE_HOG_WIDTH, APERTURE_HOG_HEIGHT);

        static Size blockSize = new Size(16, 16);
        static Size blockStride = new Size(8, 8);
        static Size cellSize = new Size(8, 8);
        static Size winStride = new Size(8, 8);
        static Size trainPadding = new Size(0, 0);
        static int nbins = 9;
        static int derivAperture = 1;
        static double winSigma = -1;
        static double L2HysThreshold = 0.2;
        static bool gammaCorrection = true;

        //static int nLevels = 64;
        //static Size testPadding = new Size(32, 32);
        //static double hitThreshold = 0;
        //static int groupThreshold = 2;
        //static double scaleStep = 1.05;
        //static bool useMeanShiftGrouping = false;


        public static List<PSM4TxSample> LoadSamples(string folder)
        {
            //Load all samples from all the folders
            List<PSM4TxSample> samples = new List<PSM4TxSample>(); List<string> missing_xml_samples = new List<string>();

            string[] files = Directory.EnumerateFiles(folder)
                .Where(file => file.ToLower().EndsWith(".bmp") || file.ToLower().EndsWith(".jpg"))
                .ToArray();

            for (int k = 0; k < files.Length; k++)
            {
                string img_file = files[k].ToLower();
                string xml_file = img_file.EndsWith("bmp") ? img_file.Replace(".bmp", "_data.xml") : img_file.Replace(".jpg", "_data.xml");

                if (!File.Exists(xml_file))
                {
                    missing_xml_samples.Add(img_file); continue;
                }

                PSM4TxSample sample = new PSM4TxSample(img_file);

                //  .// Means descendants, which includes children of children (and so forth).
                //  ./ Means direct children.
                //If a XPath starts with a / it becomes relative to the root of the document; 
                //to make it relative to your own node start it with ./.
                HtmlAgilityPack.HtmlDocument doc = new HtmlAgilityPack.HtmlDocument(); doc.Load(xml_file);
                HtmlAgilityPack.HtmlNodeCollection shape_nodes = doc.DocumentNode.SelectNodes("//shape");
                foreach (HtmlAgilityPack.HtmlNode shape_node in shape_nodes)
                {
                    HtmlAgilityPack.HtmlNode blocktext_node = shape_node.SelectSingleNode(".//blocktext");
                    HtmlAgilityPack.HtmlNode text_node = blocktext_node.SelectSingleNode("./text");
                    HtmlAgilityPack.HtmlNode data_node = shape_node.SelectSingleNode(".//data");
                    HtmlAgilityPack.HtmlNode extent_node = data_node.SelectSingleNode("./extent");

                    string s_x = extent_node.GetAttributeValue("X", "");
                    string s_y = extent_node.GetAttributeValue("Y", "");
                    string s_w = extent_node.GetAttributeValue("Width", "");
                    string s_h = extent_node.GetAttributeValue("Height", "");

                    if (text_node.InnerText.ToLower() == "isolator")
                        sample.SetIsolator(s_x, s_y, s_w, s_h); //(364.144775390625, 313.13134765625)
                    else if (text_node.InnerText.ToLower() == "target")
                        sample.SetTarget(s_x, s_y, s_w, s_h); //(338.028076171875, 2204.86572265625)
                    else if (text_node.InnerText.ToLower() == "aperture")
                        sample.SetAperture(s_x, s_y, s_w, s_h); //(80.98974609375, 958.380126953125)
                }

                samples.Add(sample);
            }

            using (StreamWriter sw = new StreamWriter("missing_xml.txt"))
            {
                for (int i = 0; i < missing_xml_samples.Count; i++) sw.WriteLine(missing_xml_samples[i]);
            }

            return samples;
        }

        public static void CropAndNormalizeObjects()
        {
            List<PSM4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\PSM4-Tx\Original");

            for (int i = 0; i < samples.Count; i++)
            {
                PSM4TxSample sample = samples[i];

                Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(sample.imageFile);

                //{   //Target
                //    //Extends in X direction to make width 500 pixels
                //    double diff_x = TARGET_ORIGINAL_WIDTH - sample.targetW;
                //    //Extends in Y direction to make 2500 pixels
                //    double diff_y = TARGET_ORIGINAL_HEIGHT - sample.targetH;

                //    Rectangle rect = new Rectangle((int)(sample.targetX - diff_x / 2 + 0.5), (int)(sample.targetY - diff_y / 2 + 0.5), TARGET_ORIGINAL_WIDTH, TARGET_ORIGINAL_HEIGHT);
                //    image.ROI = rect;
                //    Emgu.CV.Image<Gray, byte> normalized = image.Resize(TARGET_HOG_WIDTH, TARGET_HOG_HEIGHT, Inter.Linear);

                //    string cropped_file = sample.imageFile.Replace("original", "Target");
                //    normalized.Save(cropped_file);
                //}

                //{   //Isolator
                //    //Extends in X direction to make width 500 pixels
                //    double diff_x = ISOLATOR_ORIGINAL_WIDTH - sample.isolatorW;
                //    //Extends in Y direction to make 500 pixels
                //    double diff_y = ISOLATOR_ORIGINAL_HEIGHT - sample.isolatorH;

                //    Rectangle rect = new Rectangle((int)(sample.isolatorX - diff_x / 2 + 0.5), (int)(sample.isolatorY - diff_y / 2 + 0.5), ISOLATOR_ORIGINAL_WIDTH, ISOLATOR_ORIGINAL_HEIGHT);
                //    image.ROI = rect;
                //    Emgu.CV.Image<Gray, byte> normalized = image.Resize(ISOLATOR_HOG_WIDTH, ISOLATOR_HOG_HEIGHT, Inter.Linear);

                //    string cropped_file = sample.imageFile.Replace("original", "Isolator");
                //    normalized.Save(cropped_file);
                //}

                {   //Aperture
                    //Extends in X direction to make width 160 pixels
                    double diff_x = APERTURE_ORIGINAL_WIDTH - sample.apertureW;
                    //Extends in Y direction to make 1200 pixels
                    double diff_y = APERTURE_ORIGINAL_HEIGHT - sample.apertureH;

                    Rectangle rect = new Rectangle((int)(sample.apertureX - diff_x / 2 + 0.5), (int)(sample.apertureY - diff_y / 2 + 0.5), APERTURE_ORIGINAL_WIDTH, APERTURE_ORIGINAL_HEIGHT);
                    image.ROI = rect;
                    Emgu.CV.Image<Gray, byte> normalized = image.Resize(APERTURE_HOG_WIDTH, APERTURE_HOG_HEIGHT, Inter.Linear);

                    string cropped_file = sample.imageFile.Replace("original", "Aperture");
                    normalized.Save(cropped_file);
                }
            }
        }

        public static void NegativeTargetPatches()
        {
            List<PSM4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\CLR4-Tx\Original");
            for (int i = 0; i < samples.Count; i++)
            {
                PSM4TxSample sample = samples[i];
                Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(sample.imageFile);
                int image_width = image.Width, image_height = image.Height;
                Random rand = new Random();

                for (int k = 0; k < 2; k++)
                {
                    int x, y, w = TARGET_ORIGINAL_WIDTH, h = TARGET_ORIGINAL_HEIGHT;
                    x = rand.Next(0, image_width); y = rand.Next(0, image_height);

                    if (x + w >= image_width || y + h >= image_height) { k--; continue; }

                    Rectangle rect = new Rectangle(x, y, w, h);
                    image.ROI = rect;
                    Emgu.CV.Image<Gray, byte> neg_image = image.Resize(TARGET_HOG_WIDTH, TARGET_HOG_HEIGHT, Inter.Linear);

                    string negative_image_file = sample.imageFile.Replace("original", @"TargetNegative");
                    negative_image_file = negative_image_file.Insert(negative_image_file.LastIndexOf('.'), "." + k.ToString());
                    string folder = Path.GetDirectoryName(negative_image_file);
                    if (!Directory.Exists(folder)) Directory.CreateDirectory(folder);
                    neg_image.Save(negative_image_file);
                }
            }
        }

        public static void NegativeIsolatorPatches()
        {
            List<PSM4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\PSM4-Tx\Original");
            for (int i = 0; i < samples.Count; i++)
            {
                PSM4TxSample sample = samples[i];
                Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(sample.imageFile);
                int image_width = image.Width, image_height = image.Height;
                Random rand = new Random();

                for (int k = 0; k < 2; k++)
                {
                    int x, y, w = ISOLATOR_ORIGINAL_WIDTH, h = ISOLATOR_ORIGINAL_HEIGHT;
                    x = rand.Next(0, image_width); y = rand.Next(0, image_height);

                    if (x + w >= image_width || y + h >= image_height) { k--; continue; }

                    Rectangle rect = new Rectangle(x, y, w, h);
                    image.ROI = rect;
                    Emgu.CV.Image<Gray, byte> neg_image = image.Resize(ISOLATOR_HOG_WIDTH, ISOLATOR_HOG_HEIGHT, Inter.Linear);

                    string negative_image_file = sample.imageFile.Replace("original", @"IsolatorNegative");
                    negative_image_file = negative_image_file.Insert(negative_image_file.LastIndexOf('.'), "." + k.ToString());
                    string folder = Path.GetDirectoryName(negative_image_file);
                    if (!Directory.Exists(folder)) Directory.CreateDirectory(folder);
                    neg_image.Save(negative_image_file);
                }
            }
        }

        public static void NegativeAperturePatches()
        {
            List<PSM4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\PSM4-Tx\Original");
            for (int i = 0; i < samples.Count; i++)
            {
                PSM4TxSample sample = samples[i];
                Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(sample.imageFile);
                int image_width = image.Width, image_height = image.Height;
                Random rand = new Random();

                for (int k = 0; k < 2; k++)
                {
                    int x, y, w = APERTURE_ORIGINAL_WIDTH, h = APERTURE_ORIGINAL_HEIGHT;
                    x = rand.Next(0, image_width); y = rand.Next(0, image_height);

                    if (x + w >= image_width || y + h >= image_height) { k--; continue; }

                    Rectangle rect = new Rectangle(x, y, w, h);
                    image.ROI = rect;
                    Emgu.CV.Image<Gray, byte> neg_image = image.Resize(APERTURE_HOG_WIDTH, APERTURE_HOG_HEIGHT, Inter.Linear);

                    string negative_image_file = sample.imageFile.Replace("original", @"ApertureNegative");
                    negative_image_file = negative_image_file.Insert(negative_image_file.LastIndexOf('.'), "." + k.ToString());
                    string folder = Path.GetDirectoryName(negative_image_file);
                    if (!Directory.Exists(folder)) Directory.CreateDirectory(folder);
                    neg_image.Save(negative_image_file);
                }
            }
        }

        public static void TrainTargetDetection()
        {
            HOGDescriptor hog = new HOGDescriptor(winSizeTarget, blockSize, blockStride, cellSize, nbins, derivAperture, winSigma, L2HysThreshold, gammaCorrection);

            //Postive samples
            List<float> pos_targets = new List<float>(); List<float[]> pos_features = new List<float[]>();
            {
                string[] files = Directory.EnumerateFiles(@"\users\jie\projects\Intel\data\CLR4-Tx\Target")
                    .Where(file => file.ToLower().EndsWith(".bmp") || file.ToLower().EndsWith(".jpg"))
                    .ToArray();

                for (int k = 0; k < files.Length; k++)
                {
                    Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(files[k]);
                    float[] feature = hog.Compute(image);
                    pos_features.Add(feature);
                    pos_targets.Add(1.0f);
                }
            }

            //Negative samples
            List<float> neg_targets = new List<float>(); List<float[]> neg_features = new List<float[]>();
            {
                string[] files = Directory.EnumerateFiles(@"\users\jie\projects\Intel\data\CLR4-Tx\TargetNegative")
                    .Where(file => file.ToLower().EndsWith(".bmp") || file.ToLower().EndsWith(".jpg"))
                    .ToArray();

                for (int k = 0; k < files.Length; k++)
                {
                    Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(files[k]);
                    float[] feature = hog.Compute(image);
                    neg_features.Add(feature);
                    neg_targets.Add(-1.0f);
                }
            }
            List<float> targets = new List<float>(); List<float[]> features = new List<float[]>();
            targets.AddRange(pos_targets); targets.AddRange(neg_targets);
            features.AddRange(pos_features); features.AddRange(neg_features);
            LibSVM.SaveInLibSVMFormat("trainTarget.txt", targets.ToArray(), features.ToArray());

        }

        public static void TrainIsolatorDetection()
        {
            HOGDescriptor hog = new HOGDescriptor(winSizeIsolator, blockSize, blockStride, cellSize, nbins, derivAperture, winSigma, L2HysThreshold, gammaCorrection);

            //Postive samples
            List<float> pos_targets = new List<float>(); List<float[]> pos_features = new List<float[]>();
            {
                string[] files = Directory.EnumerateFiles(@"\users\jie\projects\Intel\data\CLR4-Tx\Isolator")
                    .Where(file => file.ToLower().EndsWith(".bmp") || file.ToLower().EndsWith(".jpg"))
                    .ToArray();

                for (int k = 0; k < files.Length; k++)
                {
                    Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(files[k]);
                    float[] feature = hog.Compute(image);
                    pos_features.Add(feature);
                    pos_targets.Add(1.0f);
                }
            }

            //Negative samples
            List<float> neg_targets = new List<float>(); List<float[]> neg_features = new List<float[]>();
            {
                string[] files = Directory.EnumerateFiles(@"\users\jie\projects\Intel\data\CLR4-Tx\IsolatorNegative")
                    .Where(file => file.ToLower().EndsWith(".bmp") || file.ToLower().EndsWith(".jpg"))
                    .ToArray();

                for (int k = 0; k < files.Length; k++)
                {
                    Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(files[k]);
                    float[] feature = hog.Compute(image);
                    neg_features.Add(feature);
                    neg_targets.Add(-1.0f);
                }
            }
            List<float> targets = new List<float>(); List<float[]> features = new List<float[]>();
            targets.AddRange(pos_targets); targets.AddRange(neg_targets);
            features.AddRange(pos_features); features.AddRange(neg_features);
            LibSVM.SaveInLibSVMFormat("trainIsolator.txt", targets.ToArray(), features.ToArray());

        }

        public static void TrainApertureDetection()
        {
            HOGDescriptor hog = new HOGDescriptor(winSizeAperture, blockSize, blockStride, cellSize, nbins, derivAperture, winSigma, L2HysThreshold, gammaCorrection);

            //Postive samples
            List<float> pos_targets = new List<float>(); List<float[]> pos_features = new List<float[]>();
            {
                string[] files = Directory.EnumerateFiles(@"\users\jie\projects\Intel\data\PSM4-Tx\Aperture")
                    .Where(file => file.ToLower().EndsWith(".bmp") || file.ToLower().EndsWith(".jpg"))
                    .ToArray();

                for (int k = 0; k < files.Length; k++)
                {
                    Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(files[k]);
                    float[] feature = hog.Compute(image);
                    pos_features.Add(feature);
                    pos_targets.Add(1.0f);
                }
            }

            //Negative samples
            List<float> neg_targets = new List<float>(); List<float[]> neg_features = new List<float[]>();
            {
                string[] files = Directory.EnumerateFiles(@"\users\jie\projects\Intel\data\PSM4-Tx\ApertureNegative")
                    .Where(file => file.ToLower().EndsWith(".bmp") || file.ToLower().EndsWith(".jpg"))
                    .ToArray();

                for (int k = 0; k < files.Length; k++)
                {
                    Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(files[k]);
                    float[] feature = hog.Compute(image);
                    neg_features.Add(feature);
                    neg_targets.Add(-1.0f);
                }
            }
            List<float> targets = new List<float>(); List<float[]> features = new List<float[]>();
            targets.AddRange(pos_targets); targets.AddRange(neg_targets);
            features.AddRange(pos_features); features.AddRange(neg_features);
            LibSVM.SaveInLibSVMFormat("trainAperture.txt", targets.ToArray(), features.ToArray());

        }

        public static void ToSingleVector()
        {
            string svm_file = @"\users\jie\projects\Intel\data\PSM4-Tx\models\svm_model_aperture";
            string single_vector_svm_model_file = "single_vector_aperture";
            string array_name = "PSM4Tx_APERTURE_SVM";

            LibSVM svm = new LibSVM(); svm.LoadModel(svm_file);
            float[] single_vector = svm.ToSingleVector();

            using (StreamWriter sw = new StreamWriter(single_vector_svm_model_file))
            {
                sw.Write("const float " + array_name + "[] = ");
                sw.Write("{");
                for (int i = 0; i < single_vector.Length; i++)
                {
                    sw.Write("{0}f,", single_vector[i]);
                }
                sw.WriteLine("};");
            }
        }
    }
}
