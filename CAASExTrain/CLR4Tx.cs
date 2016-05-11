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
    class CLR4TxSample
    {
        public string imageFile;
        public double isolatorX, isolatorY, isolatorW, isolatorH;
        public double targetX, targetY, targetW, targetH;


        public CLR4TxSample(string image_file)
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
    };


    class CLR4Tx
    {
        static int TARGET_ORIGINAL_WIDTH = 500;
        static int TARGET_ORIGINAL_HEIGHT = 2500;
        static int TARGET_HOG_WIDTH = 40;
        static int TARGET_HOG_HEIGHT = 200;

        static int ISOLATOR_ORIGINAL_WIDTH = 500;
        static int ISOLATOR_ORIGINAL_HEIGHT = 500;
        static int ISOLATOR_HOG_WIDTH = 80;
        static int ISOLATOR_HOG_HEIGHT = 80;

        static Size winSizeTarget = new Size(TARGET_HOG_WIDTH, TARGET_HOG_HEIGHT);
        static Size winSizeIsolator = new Size(ISOLATOR_HOG_WIDTH, ISOLATOR_HOG_HEIGHT);

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


        public static List<CLR4TxSample> LoadSamples(string folder)
        {
            //Load all samples from all the folders
            List<CLR4TxSample> samples = new List<CLR4TxSample>(); List<string> missing_xml_samples = new List<string>();

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

                CLR4TxSample sample = new CLR4TxSample(img_file);

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
                        sample.SetIsolator(s_x, s_y, s_w, s_h);
                    else if (text_node.InnerText.ToLower() == "target")
                        sample.SetTarget(s_x, s_y, s_w, s_h);
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
            List<CLR4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\CLR4-Tx\Original");

            for (int i = 0; i < samples.Count; i++)
            {
                CLR4TxSample sample = samples[i];

                Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(sample.imageFile);

                {   //Target
                    //Extends in X direction to make width 500 pixels
                    double diff_x = TARGET_ORIGINAL_WIDTH - sample.targetW;
                    //Extends in Y direction to make 2500 pixels
                    double diff_y = TARGET_ORIGINAL_HEIGHT - sample.targetH;

                    Rectangle rect = new Rectangle((int)(sample.targetX - diff_x / 2 + 0.5), (int)(sample.targetY - diff_y / 2 + 0.5), TARGET_ORIGINAL_WIDTH, TARGET_ORIGINAL_HEIGHT);
                    image.ROI = rect;
                    Emgu.CV.Image<Gray, byte> normalized = image.Resize(TARGET_HOG_WIDTH, TARGET_HOG_HEIGHT, Inter.Linear);

                    string cropped_file = sample.imageFile.Replace("original", "Target");
                    normalized.Save(cropped_file);
                }

                {   //Isolator
                    //Extends in X direction to make width 500 pixels
                    double diff_x = ISOLATOR_ORIGINAL_WIDTH - sample.isolatorW;
                    //Extends in Y direction to make 500 pixels
                    double diff_y = ISOLATOR_ORIGINAL_HEIGHT - sample.isolatorH;

                    Rectangle rect = new Rectangle((int)(sample.isolatorX - diff_x / 2 + 0.5), (int)(sample.isolatorY - diff_y / 2 + 0.5), ISOLATOR_ORIGINAL_WIDTH, ISOLATOR_ORIGINAL_HEIGHT);
                    image.ROI = rect;
                    Emgu.CV.Image<Gray, byte> normalized = image.Resize(ISOLATOR_HOG_WIDTH, ISOLATOR_HOG_HEIGHT, Inter.Linear);

                    string cropped_file = sample.imageFile.Replace("original", "Isolator");
                    normalized.Save(cropped_file);
                }
            }
        }

        public static void NegativeTargetPatches()
        {
            List<CLR4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\CLR4-Tx\Original");
            for (int i = 0; i < samples.Count; i++)
            {
                CLR4TxSample sample = samples[i];
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
            List<CLR4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\CLR4-Tx\Original");
            for (int i = 0; i < samples.Count; i++)
            {
                CLR4TxSample sample = samples[i];
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

        public static void ToSingleVector()
        {
            string svm_file = @"\users\jie\projects\Intel\data\CLR4-Tx\models\svm_model_isolator";
            string single_vector_svm_model_file = "single_vector_isolator";
            string array_name = "CLR4Tx_ISOLATOR_SVM";

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

        public static void BootStrapping()
        {
            HOGDescriptor hog = new HOGDescriptor(winSizeTarget, blockSize, blockStride, cellSize, nbins, derivAperture, winSigma, L2HysThreshold, gammaCorrection);
            hog.SetSVMDetector(CLR4Tx_Target_SVM);

            List<CLR4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\CLR4-Tx\Original");
            for (int i = 0; i < samples.Count; i++)
            {
                CLR4TxSample sample = samples[i];

                Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(sample.imageFile);
                //hog.DetectMultiScale(image)
            }
        }

        static float[] CLR4Tx_Target_SVM = { -0.5837685f, -0.2970639f, -0.3171524f, -0.1762824f, -0.3305062f, -0.06494232f, -0.1619836f, -0.1994211f, -0.5937883f, -0.02993589f, 0.3902687f, 0.03481064f, 0.4785047f, 0.8810941f, 0.04911182f, -0.2419631f, -0.2578056f, -0.5502056f, -0.5343361f, -0.3762094f, -0.173176f, -0.1446234f, -0.3329849f, -0.149684f, -0.306615f, -0.4004479f, -0.6990982f, -0.2604237f, 0.5034828f, 0.5567649f, 1.279854f, 1.102452f, 0.8246278f, 0.02984759f, -0.2506914f, -0.679747f, 0.3046696f, 0.1959348f, -0.3699358f, -0.2174765f, -0.1169055f, -0.2538731f, -0.2761187f, -0.2360522f, 0.01718347f, 0.737229f, -0.1226979f, -0.5572989f, -0.5572034f, -0.6827917f, -0.3337696f, -0.3030346f, -0.1923915f, 0.8275849f, -0.1018852f, 0.4049813f, 0.2682543f, 0.8045343f, 0.7984516f, 0.07428215f, -0.1856933f, -0.3768334f, -0.3281489f, 0.3922432f, 0.3119701f, 0.2496204f, 0.1871365f, 0.09426606f, -0.1552255f, 0.05993736f, -0.181652f, 0.2462088f, 0.9613456f, -0.1910246f, -0.4454539f, -0.5579563f, -0.8397613f, -0.2657152f, -0.3009823f, -0.2245695f, 0.9708624f, 0.9892975f, -0.2265407f, -0.5164879f, -0.5989477f, -0.9076771f, -0.3027478f, -0.3568324f, -0.3871785f, 0.9454492f, 0.6015474f, 0.1333549f, 0.01828432f, -0.1866687f, -0.4523143f, -0.08400971f, 0.04977844f, -0.2619387f, 0.2615118f, 0.5759242f, -0.2622029f, -0.3016081f, -0.2990065f, -0.439249f, -0.1053041f, -0.3111063f, -0.5217863f, 0.3906493f, 1.139675f, -0.3698444f, -0.5414178f, -0.504308f, -0.7778503f, -0.3938425f, -0.4525383f, -0.3873205f, 1.059806f, 1.005501f, -0.3416634f, -0.5827561f, -0.5796896f, -0.8665003f, -0.4816833f, -0.571325f, -0.4626091f, 0.9910481f, 0.5854828f, -0.5983777f, -0.5510677f, -0.3009591f, -0.390944f, -0.06684966f, -0.4940039f, -0.5661379f, 0.6512198f, 0.4363363f, -0.543681f, -0.4891838f, -0.2627559f, -0.574275f, -0.1851558f, -0.4162115f, -0.5010058f, 0.6453025f, 0.9109496f, -0.346489f, -0.5684063f, -0.6058939f, -0.8478288f, -0.4738063f, -0.4549478f, -0.3541539f, 0.9548153f, 0.9444693f, -0.495512f, -0.7238206f, -0.8166263f, -0.9923947f, -0.4662092f, -0.2709304f, -0.08281506f, 0.977302f, 0.5342698f, -0.2949804f, -0.1548507f, -0.030571f, -0.5038751f, -0.3414248f, -0.3359581f, -0.3811527f, 0.5311009f, 0.7985675f, -0.2208519f, -0.3267114f, -0.1442052f, -0.1244835f, -0.3559707f, -0.3410974f, -0.1893537f, 0.6696368f, 0.7165022f, -0.3137412f, -0.4977763f, -0.6006441f, -0.5624485f, -0.09172894f, 0.1240415f, 0.2808623f, 0.8351249f, 0.3923141f, -0.2250077f, -0.5741891f, -0.5926651f, -0.6143537f, -0.2104867f, -0.2654802f, -0.1971096f, 0.2831491f, 0.6669068f, -0.1906168f, -0.2415728f, 0.2311588f, 0.4571218f, 0.2872202f, 0.1279486f, 0.1242917f, 0.5114096f, 0.2507356f, -0.1581995f, -0.4173812f, -0.02153464f, 0.1831499f, 0.007573266f, -0.1646442f, -0.3373745f, 0.1231652f, 0.9816344f, 0.03529636f, -0.5846772f, -0.6953571f, -0.8452302f, -0.4874141f, -0.4337433f, -0.3298705f, 0.5637428f, 1.031494f, 0.01794523f, -0.4721065f, -0.5909984f, -0.8930033f, -0.4851368f, -0.4355268f, -0.3307583f, 0.9227683f, 0.7756591f, 0.01297843f, -0.5984493f, -0.5843244f, -0.5477875f, -0.3070535f, -0.3044751f, -0.2160362f, 0.7698865f, 0.7905661f, 0.1340011f, -0.5534422f, -0.6179094f, -0.7339717f, -0.281788f, -0.3612002f, -0.1384934f, 0.8604601f, 0.9002712f, -0.1778289f, -0.4721291f, -0.4819138f, -0.8122821f, -0.4119713f, -0.3695418f, -0.2604739f, 0.8697875f, 0.7030092f, -0.3008826f, -0.6583018f, -0.755077f, -1.086445f, -0.6478488f, -0.4131038f, -0.3324946f, 0.8084322f, 0.7426696f, 0.0598295f, -0.357379f, -0.372415f, -0.6842629f, -0.2519048f, -0.2783667f, -0.1190885f, 0.7246413f, 0.684381f, 0.1474896f, -0.4117856f, -0.4721951f, -0.8802932f, -0.4242156f, -0.4600313f, -0.2281916f, 0.6360952f, 0.8653545f, -0.1931607f, -0.5896861f, -0.5747662f, -0.9836979f, -0.5575864f, -0.3653507f, -0.3338473f, 0.8834965f, 0.9182656f, -0.07665966f, -0.569334f, -0.4449321f, -0.9317173f, -0.48458f, -0.4431666f, -0.3888906f, 0.862952f, 0.8338909f, 0.2683065f, -0.392614f, -0.3773102f, -0.7088592f, -0.4417611f, -0.5370585f, -0.2715788f, 0.609419f, 0.5874249f, 0.03515872f, -0.4819888f, -0.3694825f, -0.738582f, -0.38234f, -0.4880188f, -0.2658566f, 0.6558443f, 0.9335564f, -0.0601735f, -0.5399739f, -0.4723728f, -0.9309179f, -0.4108112f, -0.4268893f, -0.3692102f, 0.7423047f, 0.6809547f, -0.1233118f, -0.6071419f, -0.5399309f, -0.8131576f, -0.40401f, -0.4040389f, -0.3822133f, 0.7767f, 0.6600164f, -0.05843776f, -0.5376565f, -0.460037f, -0.7775103f, -0.2533564f, -0.4186953f, -0.2623059f, 0.8167638f, 0.4412679f, 0.01549803f, -0.4942821f, -0.5457378f, -0.7418337f, -0.278915f, -0.4002086f, -0.2255761f, 0.686361f, 0.8971409f, -0.1356573f, -0.4477408f, -0.3910937f, -0.5406105f, -0.329302f, -0.3616798f, -0.3063508f, 0.8952843f, 0.7663699f, -0.2982514f, -0.5206532f, -0.6122575f, -0.8462282f, -0.5063394f, -0.5208061f, -0.2849141f, 0.8007767f, 0.5283521f, -0.06468821f, -0.3741266f, -0.3299172f, -0.5946665f, -0.2682464f, -0.3269583f, -0.2228223f, 0.628648f, 0.5604864f, -0.09184813f, -0.5476789f, -0.4892863f, -0.7506337f, -0.4510658f, -0.6232117f, -0.5775903f, 0.628697f, 0.9232438f, -0.06991016f, -0.3636753f, -0.5348211f, -0.8111339f, -0.4346601f, -0.4233946f, -0.2244173f, 0.9244576f, 0.953762f, -0.006947681f, -0.3903709f, -0.460562f, -0.9822707f, -0.5539711f, -0.5052073f, -0.1678515f, 0.8743094f, 0.7126962f, 0.1001955f, -0.4462188f, -0.4272082f, -0.6655633f, -0.4249282f, -0.5791509f, -0.5300852f, 0.7072774f, 0.6594317f, 0.198381f, -0.5140328f, -0.4703186f, -0.7655455f, -0.5071797f, -0.5539256f, -0.2792471f, 0.6042346f, 1.137408f, -0.04199419f, -0.4520046f, -0.4243762f, -0.8308617f, -0.5570859f, -0.4446432f, -0.1111434f, 1.031312f, 0.8660516f, -0.02356565f, -0.465223f, -0.5134907f, -0.9701627f, -0.517489f, -0.4649671f, -0.1729887f, 0.8259668f, 0.8408475f, -0.01233418f, -0.5433182f, -0.4545632f, -0.6998613f, -0.406989f, -0.4412781f, -0.02661809f, 0.7927541f, 0.5702062f, -0.102568f, -0.7172472f, -0.5755727f, -0.9478886f, -0.3936703f, -0.4164326f, -0.04658643f, 0.6672829f, 0.7296523f, -0.03389766f, -0.2365299f, -0.3894963f, -0.7659184f, -0.3549857f, -0.3950142f, -0.1152397f, 0.9392534f, 0.5988746f, -0.07672522f, -0.3525958f, -0.7348756f, -1.001653f, -0.6953976f, -0.7225203f, -0.3002277f, 0.7180296f, 0.7848641f, 0.07948743f, -0.4081655f, -0.4252537f, -0.7215501f, -0.2635981f, -0.3545035f, -0.0573111f, 0.8871237f, 0.8284032f, 0.2024407f, -0.3606712f, -0.5275739f, -0.7442619f, -0.4519627f, -0.5172608f, -0.1034434f, 0.7854611f, 0.7084055f, -0.1472823f, -0.4155332f, -0.6377855f, -0.8454139f, -0.4636959f, -0.5597512f, -0.3307166f, 0.6021602f, 0.6313462f, -0.3020107f, -0.4055749f, -0.3029066f, -0.7016288f, -0.1599477f, -0.331647f, -0.2861353f, 0.6502628f, 0.9217115f, 0.04747208f, -0.3322753f, -0.319062f, -0.5237483f, -0.3378584f, -0.4356185f, -0.1537588f, 0.8980235f, 0.7014954f, -0.2256628f, -0.3858635f, -0.1301755f, -0.5056554f, -0.1259229f, -0.3428343f, -0.225269f, 0.7304677f, 0.5690454f, -0.3326747f, -0.3166069f, -0.2471222f, -0.6736479f, -0.2471676f, -0.370329f, -0.3189041f, 0.6876978f, 0.570737f, -0.1911243f, -0.4324859f, -0.3829824f, -0.8083383f, -0.3902305f, -0.4008861f, -0.4662783f, 0.5104426f, 0.8647265f, -0.2277888f, -0.3567438f, -0.1564502f, -0.5400524f, -0.2440706f, -0.270753f, -0.280285f, 0.8190267f, 0.7477869f, -0.0779872f, -0.5759031f, -0.5151746f, -0.7133979f, -0.3254495f, -0.2937374f, -0.3780213f, 0.7247192f, 0.6042402f, -0.1302429f, -0.4537879f, -0.3486361f, -0.6288491f, -0.3132687f, -0.2450711f, -0.4080854f, 0.3431573f, 0.4041681f, -0.1685707f, -0.5379449f, -0.4167614f, -0.6002505f, -0.3314942f, -0.2902662f, -0.2384264f, 0.392882f, 0.6904411f, -0.02767774f, -0.567068f, -0.4813271f, -0.6125233f, -0.2883975f, -0.2317416f, -0.2517756f, 0.7299869f, 0.4422307f, -0.07283744f, -0.5279568f, -0.4204498f, -0.665333f, -0.3287213f, -0.3134984f, 0.01899203f, 0.5521609f, 0.4919611f, -0.2426119f, -0.5095466f, -0.4269662f, -0.5833997f, -0.371428f, -0.3024029f, -0.08743484f, 0.5004745f, 0.5671231f, -0.3651077f, -0.5833934f, -0.517509f, -0.7191679f, -0.3864514f, -0.3898574f, -0.02067912f, 0.6396788f, 0.7977378f, -0.01424791f, -0.41799f, -0.3728049f, -0.6818735f, -0.3579472f, -0.4973203f, -0.1755464f, 0.7631655f, 0.6466831f, 0.08356413f, -0.1798216f, -0.3766435f, -0.6611015f, -0.2659934f, -0.5229552f, -0.3044183f, 0.6378212f, 0.3658057f, -0.2421389f, -0.2302829f, -0.3560824f, -0.5567828f, -0.2322608f, -0.2084796f, -0.1579867f, 0.3801527f, 0.8554277f, -0.3319766f, -0.2204077f, -0.6441894f, -0.8922808f, -0.380649f, -0.1899192f, -0.02165059f, 0.9333135f, 0.3214579f, -0.1199556f, -0.00346747f, -0.01489882f, 0.04837583f, 0.3576103f, 0.07628539f, -0.1710661f, 0.3989401f, 0.6024888f, -0.3815993f, -0.1672643f, -0.3130685f, -0.1404498f, 0.1808187f, 0.2330139f, 0.2478846f, 0.8030525f, 0.9463512f, -0.4826572f, -0.6241624f, -0.6783419f, -0.9304782f, -0.4091845f, -0.323148f, 0.02552018f, 0.9992181f, 0.676145f, -0.6162013f, -0.6206505f, -0.456373f, -0.7021016f, -0.2992288f, -0.4275402f, -0.2816025f, 0.6733921f, 0.7740987f, -0.2335858f, -0.5419411f, -0.5498286f, -0.3949306f, -0.05024628f, -0.05750483f, 0.2640833f, 0.8726467f, 0.5055276f, -0.239448f, -0.2345337f, -0.2529684f, -0.4080304f, 0.05127465f, 0.004608687f, 0.01466167f, 0.6528711f, 0.9464523f, -0.5286204f, -0.5358958f, -0.5071876f, -0.6824968f, -0.3580656f, -0.4168973f, -0.3084076f, 1.035548f, 0.7164562f, -0.7659178f, -0.7646242f, -0.6960743f, -0.8285992f, -0.5212436f, -0.4351064f, -0.2082011f, 0.9150871f, 0.7737113f, -0.3306227f, -0.2127096f, -0.2980446f, -0.5207689f, 0.02046739f, -0.1948531f, -0.1316606f, 0.8917382f, 0.6639328f, -0.4764771f, -0.6621935f, -0.4603693f, -0.39276f, -0.1670118f, -0.4329919f, -0.2422652f, 0.8003708f, 0.9066311f, -0.5592308f, -0.583882f, -0.436891f, -0.6457233f, -0.4008912f, -0.4354247f, -0.2479723f, 0.8786514f, 1.013983f, -0.2149598f, -0.5337934f, -0.4478002f, -0.7820974f, -0.4835244f, -0.6380336f, -0.4002177f, 0.8941964f, 0.7887002f, -0.3667613f, -0.6617897f, -0.5353865f, -0.608909f, -0.3976082f, -0.5046732f, -0.1906018f, 0.961224f, 0.9726734f, -0.1836059f, -0.6611687f, -0.4830263f, -0.8089774f, -0.6069437f, -0.6489973f, -0.3456911f, 0.9208478f, 1.003489f, -0.06989784f, -0.3987336f, -0.4886649f, -0.7904534f, -0.4731596f, -0.5815744f, -0.2525905f, 1.009964f, 0.5005342f, -0.3331555f, -0.4170092f, -0.2903408f, 0.08291782f, -0.4308003f, -0.4826187f, -0.1242073f, 0.9163365f, 0.8415489f, -0.08592485f, -0.4573031f, -0.4488837f, -0.2890982f, -0.4492362f, -0.4740457f, -0.2188652f, 0.9120312f, 0.4786973f, -0.2969632f, -0.4957375f, -0.01007166f, 0.4824084f, -0.01954962f, -0.2532945f, -0.1674662f, 0.7225349f, 0.3541715f, -0.0948053f, -0.1919759f, 0.1879905f, 1.066341f, -0.345165f, -0.1492244f, 0.1648502f, 0.5526191f, 0.05919115f, 0.1084581f, 0.0002691522f, -0.06773332f, -0.8075293f, -0.8555043f, -0.6172274f, -0.3330497f, -0.1038541f, 0.4232458f, -0.08410812f, -0.2803905f, 0.7668298f, 1.157172f, 0.3306153f, 0.1590993f, 0.2609395f, 0.5466339f, -0.01055068f, 0.01805632f, -0.1556962f, -0.3953927f, -0.9507981f, -0.7325884f, -0.3362201f, -0.009262308f, 0.03464979f, -0.4998451f, -0.4150488f, -0.266954f, -0.2621183f, -0.3746245f, -0.2361f, -0.3511473f, -0.4704605f, -0.5337691f, -0.6643113f, -0.2945141f, 0.02026247f, 0.8426229f, 1.109469f, 1.27032f, 0.3584919f, -0.4378907f, -0.6027182f, -0.3947539f, -0.2737746f, -0.3320295f, -0.3163723f, -0.5277173f, -0.3109511f, -0.499333f, -0.5576651f, -0.5764678f, -0.5226457f, -0.265128f, -0.2629453f, 0.2168583f, 0.7952473f, 1.448376f, 0.2993687f, -0.5837727f, -0.4939357f, -0.5617336f, -0.1331471f, 0.04603995f, 0.5943114f, 0.8034454f, 0.6533965f, 0.1226802f, -0.4838095f, -0.6834228f, -0.296665f, 0.03664117f, 0.2926071f, 0.4445226f, 0.7139529f, 0.2913273f, 0.1012966f, -0.2026237f, -0.439664f, -0.3973532f, -0.1675085f, -0.1654163f, 0.2638516f, 0.5669738f, 0.9854702f, 0.1082869f, -0.5501949f, -0.4932732f, 0.03272054f, 0.05312316f, 0.04098274f, 0.267332f, 0.6134102f, 0.5852662f, 0.007460885f, -0.05316569f, -0.02479556f, -0.06949605f, 0.1942947f, 0.2738183f, 0.1216789f, 0.3087299f, 0.3633474f, 0.4180255f, 0.05302863f, -0.1602595f, -0.34206f, -0.207899f, 0.02248442f, 0.1070431f, 0.2830426f, 0.3175056f, 0.137324f, -0.3755626f, -0.4173984f, 0.5453914f, 0.340095f, 0.02411142f, -0.03785297f, 0.2328458f, 0.4088171f, 0.2193044f, 0.188726f, 0.3055378f, 0.1745638f, 0.1692806f, 0.05023671f, 0.1498415f, 0.3398048f, 0.3335582f, 0.1593636f, -0.2990908f, 0.04449332f, -0.3390427f, -0.3394328f, -0.1257738f, 0.03292976f, 0.1334294f, 0.2033078f, -0.2254565f, -0.426477f, -0.1331814f, -0.2325018f, -0.2165675f, 0.1094634f, 0.2087313f, 0.1702997f, 0.2326757f, -0.08141847f, -0.2304836f, -0.03458449f, 0.108316f, 0.1533525f, 0.3417187f, 0.4732067f, 0.3668494f, 0.2019358f, -0.1225374f, -0.2877364f, 0.2705368f, 0.05760385f, 0.2984971f, 0.4799664f, 0.5198106f, 0.4205629f, 0.3551428f, -0.1647826f, -0.1166856f, 0.1645988f, -0.3561364f, -0.3074144f, 0.2176275f, 0.3467645f, 0.2993634f, -0.03568579f, -0.2123225f, -0.2878624f, -0.3310611f, 0.0009204932f, -0.204618f, 0.0002667718f, 0.4960859f, 0.6235639f, 0.008144319f, -0.3161468f, -0.437158f, -0.2397733f, -0.1081802f, -0.05090078f, 0.3029916f, 0.5216058f, 0.4934664f, 0.1915925f, -0.2431935f, -0.2958287f, -0.0864661f, 0.02045781f, 0.09343889f, 0.02729638f, 0.7175729f, 0.561171f, 0.2282184f, -0.1928706f, -0.4044043f, -0.2356803f, -0.02940285f, -0.2171568f, -0.1753719f, 0.7524547f, 0.7204974f, 0.07460645f, -0.2788629f, -0.5198368f, -0.2501834f, 0.2101418f, -0.3392994f, -0.3573106f, 0.25309f, 0.2804898f, -0.3162914f, -0.4379311f, -0.6199418f, 0.1358085f, 0.0318453f, 0.017542f, -0.09551569f, 0.7121305f, 0.5416407f, 0.2639964f, -0.219254f, -0.4701157f, -0.1549623f, 0.4897024f, -0.161293f, -0.369608f, 0.1440269f, 0.08985518f, -0.3437326f, -0.4906336f, -0.4675421f, 0.4687684f, 0.8614042f, -0.3122876f, -0.5813786f, -0.5312339f, -0.6047383f, -0.4275796f, -0.4148233f, -0.3591433f, 0.9390485f, 1.024202f, -0.05600418f, -0.606577f, -0.578145f, -0.7067564f, -0.3576761f, -0.4919674f, -0.3305218f, 0.9920151f, 0.9768205f, -0.1880872f, -0.5721746f, -0.5838497f, -0.8276599f, -0.6594485f, -0.596146f, -0.3224882f, 0.9588898f, 1.014482f, -0.09188264f, -0.6429667f, -0.6585177f, -0.9210936f, -0.6314028f, -0.6467118f, -0.25942f, 1.019043f, 0.925384f, -0.1170921f, -0.5163901f, -0.4956577f, -0.7391308f, -0.3476961f, -0.4825613f, -0.2790715f, 0.7434602f, 0.937739f, 0.09508355f, -0.5011202f, -0.4839543f, -0.9358553f, -0.5737265f, -0.7562195f, -0.3212895f, 0.6219071f, 0.9332862f, -0.1607576f, -0.4498408f, -0.5727451f, -0.9194257f, -0.525813f, -0.6184665f, -0.2075111f, 0.970148f, 0.9205632f, 0.1366875f, -0.257437f, -0.5974118f, -1.168804f, -0.6452416f, -0.7196609f, -0.2591596f, 0.9006977f, 1.057715f, 0.2831378f, -0.2448507f, -0.2733659f, -0.7199881f, -0.494172f, -0.6664665f, -0.2946964f, 0.4470275f, 0.8245789f, 0.0627438f, -0.3201427f, -0.3604372f, -0.8067663f, -0.6196629f, -0.6950513f, -0.5057333f, 0.7270321f, 1.068835f, 0.4790729f, -0.09970569f, -0.4192001f, -0.9286262f, -0.5889906f, -0.6565967f, -0.291443f, 0.944631f, 0.7737495f, 0.3577153f, -0.4757363f, -0.7298125f, -1.116661f, -0.7654589f, -0.8083385f, -0.5988236f, 0.8664383f, 1.003948f, -0.08369935f, -0.3898543f, -0.4000377f, -0.7292118f, -0.3796233f, -0.492261f, -0.3299349f, 0.7691507f, 0.8070536f, -0.02724846f, -0.3712234f, -0.4018247f, -0.8628545f, -0.3438571f, -0.4557981f, -0.296807f, 0.5373007f, 0.9843655f, 0.1169984f, -0.6283047f, -0.6698843f, -0.9759153f, -0.6347789f, -0.7114155f, -0.4001126f, 1.090572f, 0.8644662f, 0.001091272f, -0.5257189f, -0.4352056f, -1.078863f, -0.6946692f, -0.71971f, -0.3795427f, 0.8068536f, 0.8941046f, 0.1455697f, -0.1629779f, -0.2061413f, -0.4687707f, -0.2262919f, -0.2710717f, -0.1662869f, 0.3901699f, 0.8013526f, 0.198515f, -0.4030622f, -0.3977221f, -0.5843578f, -0.4447946f, -0.577192f, -0.5531062f, 0.4566694f, 0.9204709f, 0.2139867f, -0.347687f, -0.3014566f, -0.661221f, -0.4163663f, -0.468897f, -0.3726143f, 0.895691f, 0.649661f, 0.2074486f, -0.5767053f, -0.6190944f, -0.9588334f, -0.7386399f, -0.7601421f, -0.508531f, 0.4212983f, 0.9507303f, 0.2536218f, -0.3538017f, -0.3025666f, -0.5299649f, -0.400749f, -0.5942007f, -0.5242518f, 0.5591925f, 0.8128709f, 0.2909506f, -0.2562909f, -0.2157196f, -0.506943f, -0.3598979f, -0.5089027f, -0.4678752f, 0.7202975f, 0.6919062f, 0.056104f, -0.4395157f, -0.4314634f, -0.8730577f, -0.7274194f, -0.7686048f, -0.5116425f, 0.4239467f, 0.7190562f, 0.006282784f, -0.4023927f, -0.2446562f, -0.6464372f, -0.5055386f, -0.6401834f, -0.5311273f, 0.6584119f, 0.9820041f, -0.1353201f, -0.3408011f, -0.2957581f, -0.4767947f, -0.3621384f, -0.4321906f, -0.2730243f, 0.7873322f, 0.8294165f, -0.3273261f, -0.5729532f, -0.6085083f, -0.6983762f, -0.3959116f, -0.398766f, -0.1779851f, 0.876125f, 0.8154714f, -0.07441942f, -0.3933553f, -0.2695472f, -0.574076f, -0.4396296f, -0.5302352f, -0.4110499f, 0.5207453f, 0.5991233f, -0.1586827f, -0.646157f, -0.6504506f, -0.9402988f, -0.5709437f, -0.5184959f, -0.4400402f, 0.6839522f, 0.7522863f, -0.406491f, -0.5895959f, -0.5555913f, -0.627075f, -0.3182435f, -0.3294414f, -0.2663405f, 0.9121557f, 0.8655897f, -0.1616854f, -0.3972255f, -0.4452956f, -0.6419505f, -0.4646331f, -0.4927539f, -0.396018f, 0.8310034f, 0.5987245f, -0.3361392f, -0.6622383f, -0.6538491f, -0.825866f, -0.5246961f, -0.4715913f, -0.4292293f, 0.8187703f, 0.7202986f, -0.302471f, -0.5433321f, -0.6262752f, -0.8132264f, -0.6421179f, -0.5674528f, -0.4074256f, 0.8127975f, 0.7899331f, -0.2303095f, -0.3745689f, -0.3552841f, -0.6000524f, -0.479704f, -0.516553f, -0.4845623f, 0.9576954f, 0.6778421f, -0.4287372f, -0.4038087f, -0.2817393f, -0.6621553f, -0.4236361f, -0.4552038f, -0.4327345f, 0.7762927f, 0.8633452f, -0.3074648f, -0.571412f, -0.603891f, -0.7776868f, -0.6366246f, -0.5898883f, -0.5642085f, 0.9609812f, 0.7854262f, -0.2970304f, -0.5326017f, -0.5596836f, -0.8725542f, -0.699876f, -0.7432041f, -0.6418513f, 0.7608269f, 0.7903183f, -0.4111817f, -0.4543411f, -0.2843138f, -0.5952091f, -0.4001318f, -0.3746358f, -0.3613663f, 0.7790229f, 0.7841374f, -0.4732603f, -0.6566511f, -0.5456539f, -0.6992145f, -0.516345f, -0.4378269f, -0.4459332f, 0.6176712f, 0.9092041f, -0.2162654f, -0.4898658f, -0.5172076f, -0.8006649f, -0.6185226f, -0.6093675f, -0.3668091f, 0.9375638f, 0.9305092f, -0.3118546f, -0.6272755f, -0.699157f, -0.9291119f, -0.6604373f, -0.5470272f, -0.3265523f, 0.9195095f, 0.7421181f, -0.4326747f, -0.5516111f, -0.4262945f, -0.5724427f, -0.3625312f, -0.3437089f, -0.3501338f, 0.578733f, 0.6650956f, -0.4545758f, -0.6255897f, -0.3993044f, -0.6171091f, -0.3737448f, -0.4140636f, -0.2879097f, 0.5698258f, 0.9539591f, -0.1841378f, -0.6115856f, -0.5461144f, -0.7721279f, -0.4279104f, -0.4326755f, -0.2877682f, 0.8923942f, 0.822f, -0.1627922f, -0.645503f, -0.5117992f, -0.7905923f, -0.3884253f, -0.4713749f, -0.2997737f, 0.7227976f, 0.8559228f, -0.2637194f, -0.5642158f, -0.4467864f, -0.5880239f, -0.2799846f, -0.5244763f, -0.3772646f, 0.6028075f, 0.6893294f, -0.02082762f, -0.317503f, -0.3612894f, -0.5714976f, -0.2381157f, -0.5280865f, -0.3943772f, 0.5624416f, 0.9201845f, -0.08197512f, -0.5131019f, -0.5181697f, -0.7248082f, -0.4055582f, -0.4896314f, -0.1979672f, 0.7893891f, 0.6400095f, 0.0177891f, -0.5787525f, -0.5512398f, -0.6472647f, -0.5070682f, -0.4890175f, -0.01602769f, 0.6436012f, 0.125668f, -0.05551796f, -0.1118056f, -0.0187835f, 0.0412807f, 0.2645155f, 0.05000419f, -0.2880599f, 0.2521223f, -0.08192471f, -0.459391f, -0.4478327f, -0.1169459f, 0.2351745f, 0.3413652f, 0.3823868f, 0.08941384f, 0.1173724f, 0.4494264f, 0.001214776f, -0.3776472f, -0.1521078f, 0.1466178f, 0.2321535f, 0.08014353f, -0.08682523f, 0.4115969f, -0.1380631f, -0.304336f, -0.5646588f, -0.05819795f, 0.2970362f, 0.7093582f, 0.4805519f, 0.1235248f, 0.05199473f, -0.02754275f, -0.2589403f, -0.5564612f, -0.08578737f, 0.4498619f, 0.2891751f, 0.1321524f, 0.1092798f, 0.2258184f, -0.05952184f, -0.1881238f, -0.3029841f, 0.09953963f, 0.5024858f, 0.3727167f, 0.1484769f, -0.06309226f, 0.1174395f, -0.2264419f, -0.3861854f, -0.5977474f, 0.01694389f, 0.366294f, 0.3638163f, 0.1658212f, 0.09055268f, 0.09079111f, -0.04552238f, -0.3174727f, -0.4826199f, 0.05351586f, 0.4950749f, 0.3589437f, 0.3163841f, 0.07180274f, 0.04683581f, -0.01013975f, -0.1391316f, -0.130771f, 0.02058759f, 0.2098514f, 0.2925397f, 0.2171762f, -0.05911021f, 0.007157616f, 0.05907495f, -0.2530399f, -0.4506699f, -0.1153223f, -0.1593705f, 0.04912056f, -0.04941245f, -0.3186426f, -0.1017875f, 0.1871697f, 0.09888848f, -0.180624f, 0.01899167f, 0.1168188f, 0.3931612f, 0.6060256f, 0.2698764f, 0.08973262f, 0.2556006f, 0.1356672f, -0.1563325f, -0.1346401f, -0.2017087f, 0.07870704f, 0.2791368f, 0.2657643f, 0.2879066f, 0.492559f, -0.2963125f, -0.5252773f, -0.2836425f, -0.1388787f, -0.2854449f, -0.4155739f, -0.2794872f, 0.4250347f, 0.5247849f, -0.2515251f, -0.5277539f, -0.1456522f, -0.2899998f, -0.5091856f, -0.3231152f, -0.2095689f, 0.4552398f, 0.498349f, -0.1031151f, -0.1963494f, -0.04084932f, -0.06107484f, -0.1571192f, -0.08007561f, 0.2279449f, 0.4166912f, 0.5508603f, -0.02957066f, -0.1216882f, 0.1692146f, -0.3748093f, -0.2871383f, 0.01786782f, 0.2292469f, 0.4129755f, 0.2990843f, -0.2524981f, -0.3851498f, -0.03209f, -0.1216879f, -0.3171898f, -0.2229393f, -0.2302759f, 0.1781527f, 0.04938123f, -0.2767112f, -0.4012947f, 0.07942459f, 0.4462106f, 0.06907511f, -0.3211472f, -0.3879003f, -0.06193677f, 0.2990529f, -0.09968646f, -0.08442178f, 0.501415f, -0.07350308f, -0.2155188f, -0.08515474f, 0.02088575f, 0.274349f, -0.01147322f, -0.2047862f, -0.3340155f, 0.4332096f, 0.4645731f, 0.125441f, -0.3174716f, -0.2664429f, -0.1180036f, -0.01601082f, -0.07892762f, -0.1888854f, 0.4201423f, 1.46058f, 0.446532f, -0.1519283f, -0.1662093f, -0.04787169f, -0.1676946f, -0.1411495f, -0.2408589f, -0.4694521f, -0.9875097f, -0.4932872f, -0.4120447f, -0.2754771f, -0.2057179f, -0.209379f, -0.0595339f, -0.1757258f, 0.7885638f, 1.454945f, 0.4375262f, -0.1692665f, -0.1330485f, -0.2477912f, -0.2186981f, -0.2563323f, -0.3576661f, -0.6696008f, -0.9900531f, -0.5410374f, -0.3648814f, -0.2071057f, -0.3384679f, -0.06766134f, 0.01858955f, -0.1830547f, -0.2346173f, -0.4797789f, -0.2266459f, -0.4526082f, -0.4940326f, -0.3417174f, -0.2217897f, -0.01307751f, -0.07653201f, 0.1386591f, 0.9008187f, 1.444127f, -0.2906813f, -0.6133171f, -0.4269181f, 0.8099029f, 0.3913292f, -0.2527777f, -0.4668378f, -0.6943769f, -0.3929884f, -0.4976482f, -0.3572139f, 0.1983762f, 0.07110187f, 0.1786755f, -0.194639f, -0.1158867f, 0.8833823f, 1.192759f, -0.2499478f, -0.4533577f, -0.4111106f, -0.1899988f, -0.04451023f, 0.1003402f, 0.330234f, 0.7564837f, 1.240503f, -0.09623748f, -0.4467517f, -0.2680451f, 0.3033566f, 0.1905457f, 0.08222369f, 0.1138363f, 0.3847273f, 0.4845374f, 0.03142963f, 0.06267768f, 0.2919348f, -0.50179f, -0.1586696f, -0.02503744f, 0.2517571f, 0.6916184f, 1.027347f, -0.09982366f, -0.5636994f, -0.6698371f, -0.2699899f, -0.0755029f, -0.02760355f, -0.004953217f, 0.2924708f, 0.5566747f, -0.1197897f, -0.4001795f, -0.4814126f, 0.9868883f, 0.5832653f, 0.03996098f, -0.09515076f, -0.143902f, 0.2280457f, 0.06454965f, 0.3196592f, 0.81792f, 0.7012642f, 0.5413861f, 0.2382676f, 0.209294f, 0.02565245f, 0.2171135f, 0.03871769f, -0.1366021f, 0.5705996f, -0.0002219751f, 0.08110721f, -0.03279516f, -0.1676544f, -0.2760312f, 0.08745224f, -0.1258314f, -0.1100865f, -0.2441854f, -0.0933958f, 0.158732f, 0.04208066f, -0.07437577f, -0.333197f, -0.2401808f, -0.2319409f, -0.1326114f, -0.3604535f, 0.5676606f, 0.2939195f, 0.3054335f, 0.3984338f, 0.2172804f, 0.1391025f, -0.01319073f, -0.08925899f, 0.5853707f, 0.5351398f, 0.293939f, 0.2501375f, 0.3128269f, 0.266188f, 0.2913588f, 0.0216237f, 0.05931535f, 0.6117441f, -0.06610607f, 0.138479f, -0.01372743f, -0.1387596f, -0.3615355f, -0.1701052f, -0.2041476f, -0.1667507f, -0.3123895f, -0.04002376f, -0.1757273f, -0.2128272f, -0.3335012f, -0.4948845f, 0.0342958f, -0.02164803f, -0.01951623f, -0.09563287f, 0.3558444f, 0.07398695f, 0.1044402f, 0.2284466f, 0.362409f, 0.2823107f, 0.1439108f, 0.08634058f, 0.4189657f, 0.2174077f, 0.1597399f, 0.1503527f, 0.5835986f, 0.4572716f, 0.3083276f, 0.136059f, -0.01435447f, 0.04756462f, -0.14325f, -0.1736694f, -0.2200667f, -0.2983646f, -0.416249f, -0.06547521f, -0.1092834f, -0.2159747f, -0.1691688f, -0.251314f, 0.02749964f, 0.1084585f, 0.6242334f, 0.378933f, -0.01804825f, -0.2854815f, -0.4561052f, -0.340408f, 0.1158852f, -0.03960809f, -0.0874583f, 0.5397284f, 0.4664778f, 0.02196409f, -0.1645913f, -0.3705943f, -0.04723924f, 0.8347055f, -0.05066681f, -0.2640287f, 0.06810341f, -0.08892439f, -0.3010458f, -0.3578246f, -0.4175069f, 0.7497802f, 0.09534148f, -0.1969689f, 0.02108897f, 0.7226599f, 0.433456f, -0.1310484f, -0.3514096f, -0.6601594f, -0.4208898f, 0.612026f, 0.2162103f, -0.05167871f, 0.1884486f, -0.04518186f, -0.3636926f, -0.4737822f, -0.7077451f, 0.3930504f, 1.09156f, -0.1107196f, -0.3994612f, -0.4226078f, -0.6620127f, -0.5100148f, -0.4403622f, -0.321823f, 1.039139f, 1.094425f, -0.04935579f, -0.3997331f, -0.5092404f, -0.7668529f, -0.5363006f, -0.5291536f, -0.2877744f, 1.01762f, 0.8012332f, 0.3292055f, -0.2023976f, -0.4242247f, -0.6867657f, -0.4955325f, -0.5382152f, -0.4762594f, 0.666078f, 0.7547167f, 0.3299074f, -0.2524064f, -0.5503962f, -0.8478675f, -0.5449881f, -0.6919301f, -0.4671189f, 0.573086f, 1.033372f, -0.1119617f, -0.3755864f, -0.447464f, -0.6714815f, -0.3993091f, -0.5041842f, -0.2339646f, 0.9850462f, 1.023558f, -0.1842882f, -0.407783f, -0.5267441f, -0.8085806f, -0.4126933f, -0.5169343f, -0.3053058f, 0.9677169f, 0.7026504f, 0.0288325f, -0.4498144f, -0.5513351f, -0.8157736f, -0.4749736f, -0.5973179f, -0.3173797f, 0.5301387f, 0.5309424f, -0.308165f, -0.6293535f, -0.6031586f, -0.8456931f, -0.5300951f, -0.6078608f, -0.1806434f, 0.5161402f, 1.176695f, -0.01058168f, -0.2969833f, -0.4389485f, -0.6985809f, -0.4358023f, -0.5084f, -0.3911066f, 1.042967f, 0.9750791f, 0.03949498f, -0.5413417f, -0.6589267f, -0.8629925f, -0.5686262f, -0.7384263f, -0.5705833f, 0.8856719f, 0.7891327f, -0.2124501f, -0.4581656f, -0.4449316f, -0.6589978f, -0.4337652f, -0.5312238f, -0.3123752f, 0.674668f, 0.8005983f, -0.1432605f, -0.4437892f, -0.4664375f, -0.8060818f, -0.4346497f, -0.7091846f, -0.4520916f, 0.5826601f, 0.9618998f, -0.1210263f, -0.5689403f, -0.5286259f, -0.869548f, -0.5642522f, -0.6031871f, -0.432227f, 0.9575742f, 0.8971646f, -0.06551404f, -0.4797127f, -0.350695f, -0.8651691f, -0.5411359f, -0.621028f, -0.4669726f, 0.7704195f, 0.7096678f, -0.08042298f, -0.424015f, -0.4537664f, -0.833589f, -0.3211528f, -0.5285323f, -0.3480596f, 0.6213533f, 0.4869972f, 0.06251784f, -0.4714177f, -0.4531719f, -0.8671095f, -0.3811488f, -0.5921588f, -0.3258532f, 0.477208f, 0.9093763f, 0.021293f, -0.4595558f, -0.3185197f, -0.6517625f, -0.3543167f, -0.4823011f, -0.426441f, 0.8204077f, 0.6227642f, 0.09577696f, -0.6056568f, -0.6371849f, -0.9675654f, -0.5950208f, -0.6603405f, -0.5753165f, 0.4722984f, 0.6526436f, 0.08959178f, -0.353192f, -0.2339501f, -0.5911966f, -0.210888f, -0.3994042f, -0.1773514f, 0.6863484f, 0.5701954f, 0.2046053f, -0.3370241f, -0.4062232f, -0.8681162f, -0.4607147f, -0.5053461f, -0.2949199f, 0.5008364f, 0.7140884f, 0.1675563f, -0.4643896f, -0.5198164f, -0.7640949f, -0.5800563f, -0.6254272f, -0.4381912f, 0.5087928f, 0.7504959f, 0.1236938f, -0.4829792f, -0.3171192f, -0.6626815f, -0.5249966f, -0.5828915f, -0.4507161f, 0.7090222f, 0.62593f, 0.3287185f, -0.2759088f, -0.3269813f, -0.7900937f, -0.4519256f, -0.5011639f, -0.1723566f, 0.6471317f, 0.5590829f, 0.0753625f, -0.3306979f, -0.2399953f, -0.6667949f, -0.5007564f, -0.5862854f, -0.4483773f, 0.5173444f, 0.8488569f, 0.2250418f, -0.3348683f, -0.1763541f, -0.4891122f, -0.408746f, -0.5321724f, -0.3925841f, 0.6810737f, 0.6415855f, 0.2176285f, -0.4587023f, -0.4562304f, -0.8830569f, -0.5952696f, -0.6902933f, -0.5799856f, 0.6386091f, 0.7255026f, 0.0681226f, -0.2274299f, -0.1175905f, -0.3888435f, -0.4061201f, -0.5659835f, -0.4871869f, 0.6649222f, 0.6583868f, 0.07780142f, -0.3226761f, -0.2742729f, -0.7047079f, -0.5674776f, -0.6385917f, -0.4096412f, 0.5854381f, 0.8146725f, -0.05899416f, -0.4746742f, -0.5398186f, -0.806954f, -0.4323965f, -0.5181231f, -0.4210336f, 0.816746f, 0.9352797f, -0.2189384f, -0.5809835f, -0.617781f, -0.7997714f, -0.4948104f, -0.4912804f, -0.2207886f, 0.9607728f, 0.6574506f, -0.0583055f, -0.4250652f, -0.432835f, -0.7285457f, -0.3679011f, -0.4065539f, -0.1630139f, 0.5726033f, 0.7104375f, -0.1033607f, -0.6278986f, -0.5883396f, -0.6303121f, -0.2460056f, -0.363388f, -0.001204513f, 0.7395812f, 1.142419f, -0.2000737f, -0.5902874f, -0.5715255f, -0.7163092f, -0.4523674f, -0.3832435f, -0.2254012f, 1.225649f, 1.08107f, -0.05270873f, -0.5797074f, -0.610054f, -0.8377714f, -0.580547f, -0.5517248f, -0.2852022f, 1.073716f, 0.7287019f, -0.01793747f, -0.5255364f, -0.562655f, -0.6912457f, -0.3787638f, -0.395403f, -0.03295462f, 0.7106132f, 0.6669574f, -0.04642449f, -0.5611998f, -0.6988494f, -0.9377821f, -0.5723429f, -0.4524155f, 0.1072802f, 0.670634f, 1.091798f, 0.04596192f, -0.4158593f, -0.5880883f, -0.7968173f, -0.5432794f, -0.5174826f, -0.1221003f, 1.106549f, 1.089603f, -0.06066767f, -0.43954f, -0.6068395f, -0.8430296f, -0.5216043f, -0.504663f, -0.1908386f, 1.040593f, 0.6073352f, -0.1258361f, -0.4262545f, -0.5835002f, -0.8252106f, -0.5827542f, -0.4605797f, 0.1025362f, 0.61966f, 0.670867f, -0.07208633f, -0.3958045f, -0.4497123f, -0.8959071f, -0.5603786f, -0.5275838f, -0.2106489f, 0.5284075f, 0.8955848f, 0.07554888f, -0.4809476f, -0.4323885f, -0.8253487f, -0.4203423f, -0.4108604f, -0.2478809f, 0.7638294f, 0.7879537f, 0.1644657f, -0.5942329f, -0.4344452f, -0.667145f, -0.3383696f, -0.4164679f, -0.2343508f, 0.7691891f, 0.7092847f, 0.01455861f, -0.3871269f, -0.3096548f, -0.9030023f, -0.5057632f, -0.4379086f, -0.2021985f, 0.5104563f, 0.6017665f, 0.1010564f, -0.68803f, -0.3682803f, -0.6494793f, -0.3070914f, -0.4285659f, -0.1129964f, 0.6799363f, 0.9297673f, 0.2339198f, -0.2777652f, -0.4103361f, -0.4322933f, -0.3985381f, -0.4478062f, -0.1150146f, 0.8275592f, 0.6985883f, 0.2449459f, -0.386006f, -0.4226094f, -0.562627f, -0.5573556f, -0.4853162f, -0.1198187f, 0.7059754f, 0.6751888f, 0.2072827f, -0.4731775f, -0.4120353f, -0.5482311f, -0.3895057f, -0.4353153f, -0.09806028f, 0.7222268f, 0.4683661f, 0.3538571f, -0.2665396f, -0.1896093f, -0.6952378f, -0.6232715f, -0.6079835f, -0.3127683f, 0.3248779f, 0.3231477f, -0.07373698f, -0.3283318f, -0.03288816f, 0.4206971f, 0.4106321f, -0.06439801f, -0.1114746f, 0.2380431f, -0.09871055f, -0.1208181f, -0.3441676f, 0.05137657f, 0.3316777f, 0.6104676f, 0.04563824f, -0.01696311f, 0.02713053f, 0.0353573f, 0.1595773f, -0.1128637f, 0.2286738f, 0.4244773f, 0.3146037f, -0.2871461f, -0.2590117f, -0.1579063f, -0.3880253f, -0.2226774f, -0.2598212f, 0.1291114f, 0.4004015f, 0.5313107f, -0.2524503f, -0.1517726f, -0.3985609f, 0.1894258f, 0.0942394f, -0.1581205f, 0.179633f, 0.5164956f, 0.3413109f, 0.1418696f, 0.2807581f, 0.4311182f, 0.3617609f, 0.08302867f, -0.1066884f, 0.0915463f, 0.4066044f, 0.1990158f, 0.3678353f, 0.3441709f, 0.2912841f, -0.1813409f, -0.01604322f, -0.1007324f, -0.0234995f, 0.06147254f, 0.03919545f, -0.1652842f, 0.2444687f, -0.1339151f, -0.08875766f, 0.08173519f, -0.1240199f, -0.1969203f, -0.3478263f, -0.3695206f, -0.1789473f, 0.09908128f, -0.3252725f, 0.5213448f, 0.4548095f, -0.0003436618f, 0.03779441f, 0.3937139f, 0.430392f, 0.5879704f, 0.5043493f, 0.2953585f, 0.5049707f, 0.5502611f, 0.1205255f, -0.07147749f, -0.1190937f, 0.08166913f, 0.3442276f, 0.4758698f, 0.3319031f, 0.03539946f, 0.2289372f, -0.1316371f, -0.2403978f, -0.2451841f, -0.1755983f, -0.1264436f, 0.1592573f, -0.06767257f, -0.1011862f, 0.05792713f, -0.275496f, -0.2813176f, -0.4931927f, -0.3070726f, -0.1236422f, -0.02971048f, -0.2103063f, 0.3964614f, 0.2755715f, 0.2864465f, 0.2221467f, 0.09804964f, -0.01989086f, 0.2680713f, 0.4803658f, 0.3333549f, 0.4216174f, 0.308306f, 0.3964071f, 0.5205275f, -0.01618114f, -0.0119158f, 0.1603783f, 0.5090073f, 0.4697257f, -0.1663034f, 0.1475178f, 0.1165386f, -0.02257664f, -0.3705046f, -0.2809733f, -0.1230729f, -0.008050092f, -0.3937619f, -0.2318703f, 0.09835981f, 0.2520799f, 0.002486467f, -0.3808523f, -0.2552895f, -0.3304905f, 0.008667208f, -0.2815731f, 0.1157999f, 0.01673233f, 0.1919363f, 0.4763134f, 0.09181797f, -0.1197967f, -0.08812074f, 0.07514735f, 0.1422276f, -0.1564625f, -0.1817807f, 0.01260972f, 0.831259f, 0.8348f, 0.1683624f, -0.1250514f, -0.1196168f, -0.1522969f, -0.4761605f, -0.1808904f, 0.03523438f, -0.05092896f, -0.28192f, -0.3632068f, -0.3865453f, -0.2323377f, -0.5327222f, -0.5171814f, -0.3402061f, 0.1981279f, 0.8480198f, 0.6314129f, -0.02663399f, -0.1144f, -0.1920442f, -0.4710904f, -0.3438311f, -0.2542759f, -0.08824739f, 0.9414505f, 1.295192f, 0.2468819f, -0.005948938f, -0.07410873f, -0.2964717f, -0.2062857f, -0.2716033f, -0.4815688f, -0.7414871f, -0.8197622f, -0.3188104f, -0.2371102f, -0.1377776f, -0.3267866f, -0.4747886f, -0.3945382f, 0.009071052f, 1.148116f, 1.134802f, 0.4969194f, 0.3824798f, 0.015279f, -0.2788663f, -0.2951147f, -0.4827796f, -0.5665153f, -0.7383415f, -0.6892084f, -0.006464437f, 0.0488003f, -0.08523466f, -0.2028755f, 0.4237938f, 0.06236279f, -0.2493086f, -0.3798562f, -0.6181369f, -0.3305883f, -0.3911653f, -0.2750957f, 0.2705401f, -0.09715064f, -0.1339671f, -0.2788499f, 0.2886527f, 0.5256452f, 0.8075809f, -0.2074655f, -0.3005403f, -0.1274588f, 0.4991163f, -0.101758f, -0.297344f, -0.3848392f, -0.6926904f, -0.3691392f, -0.2947541f, -0.1862143f, 0.5864012f, 0.2868329f, -0.3342476f, -0.3168409f, 0.1630294f, 0.4559471f, 0.9108734f, 0.3668146f, 0.2013963f, 0.4479179f, -0.4800142f, -0.1936927f, -0.0751604f, 0.2934915f, 0.4758221f, 0.5792518f, -0.1519142f, -0.4537565f, -0.5735782f, -0.3318188f, 0.04471635f, 0.06612293f, 0.04360357f, -0.05142825f, 0.2602297f, -0.08287933f, -0.3756301f, -0.4804256f, 0.02149765f, 0.1229668f, -0.01868984f, 0.1691814f, 0.1198913f, 0.6393507f, 0.4131609f, 0.1315329f, 0.1432958f, 0.4676723f, 0.878102f, 0.432517f, 0.2019967f, -0.1347159f, 0.3282727f, 0.3303927f, 0.2131024f, 0.389655f, -0.1817519f, -0.0347071f, -0.1767648f, -0.2881927f, -0.4054585f, -0.06909019f, -0.2243854f, -0.3355685f, -0.3711647f, 0.02288432f, 0.001589067f, -0.257725f, -0.294899f, -0.4273178f, -0.2585676f, -0.3970714f, -0.2460994f, -0.1769254f, 0.8899353f, 0.8044375f, 0.1473157f, -0.04263062f, -0.3872451f, 0.1109089f, -0.02160005f, 0.2005381f, 0.8060052f, 0.789987f, 0.9529871f, 0.09720139f, -0.2679629f, -0.5229163f, -0.2455224f, -0.278109f, 0.371197f, 0.7281749f, 0.1856035f, 0.07379276f, -0.1999147f, -0.3537992f, -0.5728174f, -0.2712737f, -0.35542f, -0.261503f, -0.09025124f, 0.255185f, -0.09592332f, -0.2345515f, -0.4578151f, -0.7066349f, -0.1305534f, -0.3338774f, -0.2869215f, 0.03535229f, 0.9644998f, 1.081374f, 0.1473334f, -0.1301064f, -0.4389681f, -0.2312027f, -0.2007139f, 0.4322543f, 0.8196205f, 0.8734778f, 0.6730969f, 0.5025189f, -0.3056085f, -0.5662529f, -0.1352209f, -0.4085454f, 0.01896541f, 0.635809f, 0.6140957f, -0.02472045f, -0.1743665f, -0.3643899f, -0.5434622f, -0.1515913f, -0.2576089f, -0.297561f, 0.2799858f, 0.8931259f, 0.1488993f, 0.002735924f, 0.02588116f, -0.2854677f, -0.2420108f, -0.3929532f, -0.4722295f, 0.2502989f, 1.033747f, 0.5176566f, 0.4742944f, -0.4178711f, -0.8345094f, -0.3257707f, -0.4377632f, -0.292753f, 0.8217161f, 0.9935936f, 0.4933374f, 0.02231548f, -0.4451061f, -1.155099f, -0.4914932f, -0.5413904f, -0.3489955f, 0.8991378f, 0.6587158f, 0.2007391f, 0.3583692f, 0.8016539f, 0.2972559f, -0.1597236f, -0.4267939f, -0.5492764f, -0.1189147f, 0.6746577f, 0.4323263f, 0.3331809f, 0.3301591f, -0.1907714f, -0.4164203f, -0.4852357f, -0.2718265f, 0.3946939f, 0.7092983f, 0.5728872f, 0.1598546f, -0.01952974f, -0.8119668f, -0.4900503f, -0.7028588f, -0.6090062f, 0.4769267f, 0.7010489f, 0.2048665f, -0.02479799f, -0.1441139f, -0.7571161f, -0.6334891f, -0.6642402f, -0.2508531f, 0.3865824f, 0.8892335f, 0.430587f, 0.05761041f, -0.2703123f, -0.5764787f, -0.506107f, -0.4651012f, -0.05583717f, 0.6661469f, 0.7719879f, 0.2174403f, -0.1131153f, -0.3844136f, -0.7099336f, -0.5068609f, -0.5715134f, -0.1375099f, 0.6088269f, 0.9573796f, 0.1100529f, -0.1091467f, -0.348602f, -0.7879337f, -0.576193f, -0.4834292f, -0.0182512f, 0.6533267f, 0.8564093f, 0.2094664f, -0.1460969f, -0.4801706f, -0.8004271f, -0.2096871f, -0.4607524f, -0.07709576f, 0.6616008f, 0.6238066f, 0.01500645f, -0.3026714f, -0.3634082f, -0.6044121f, -0.3043857f, -0.3380171f, -0.03680685f, 0.5536899f, 0.4173264f, -0.1242134f, -0.4858687f, -0.3441699f, -0.6124814f, -0.338308f, -0.3382869f, 0.1396927f, 0.3755515f, 0.7767022f, 0.1597677f, -0.349018f, -0.2826451f, -0.5954946f, 0.038605f, -0.2235051f, 0.0174692f, 0.5706688f, 0.5080342f, -0.1438884f, -0.5758184f, -0.1197547f, -0.6179081f, 0.08380524f, -0.08698898f, 0.2775486f, 0.5261577f, 0.5334924f, 0.03774635f, -0.2779989f, -0.2028305f, -0.4696544f, -0.3217637f, -0.3396567f, 0.002838273f, 0.3577118f, 0.675552f, 0.1635367f, -0.1186034f, -0.108982f, -0.5577079f, -0.3337371f, -0.4838815f, -0.06434917f, 0.5082752f, 0.5675208f, 0.1134133f, -0.1782576f, -0.1231497f, -0.5813284f, -0.2048975f, -0.3026307f, 0.02489908f, 0.575128f, 0.5749717f, 0.5579007f, 0.2370761f, -0.006725892f, -0.7205282f, -0.5091403f, -0.7674276f, -0.241459f, 0.4167463f, 0.7613128f, 0.1269535f, -0.1122283f, -0.2823873f, -0.6289532f, -0.257263f, -0.3831815f, -0.091547f, 0.7172992f, 0.6410901f, 0.2236744f, -0.2483434f, -0.4272043f, -0.667686f, -0.2665766f, -0.4032505f, -0.003700804f, 0.5919833f, 0.7988337f, 0.191829f, 0.1441633f, -0.1700747f, -0.7341592f, -0.4214707f, -0.633954f, -0.3198844f, 0.5419489f, 0.5433105f, -0.1820274f, -0.370369f, -0.5450162f, -0.8287428f, -0.477349f, -0.5807361f, -0.2424357f, 0.3866619f, 0.7103162f, 0.1417941f, -0.2311855f, -0.2374518f, -0.4333687f, -0.1390989f, -0.2641157f, 0.03191391f, 0.5946324f, 0.577436f, 0.01833451f, -0.2457038f, -0.2061377f, -0.6093144f, -0.3312025f, -0.182882f, 0.07275413f, 0.5713044f, 0.6167961f, -0.05964375f, -0.5235682f, -0.4837476f, -0.592738f, -0.3002442f, -0.360728f, -0.1702589f, 0.4130102f, 0.462501f, -0.2046071f, -0.5244139f, -0.3524517f, -0.7392852f, -0.4646564f, -0.2804883f, -0.1103289f, 0.2742128f, 0.6443632f, 0.1045145f, -0.2112272f, -0.1878466f, -0.5528551f, -0.3320284f, -0.2005233f, 0.1317061f, 0.6070346f, 0.6361187f, 0.0442764f, -0.2360203f, -0.2182225f, -0.5340852f, -0.4039692f, -0.4572538f, -0.2723586f, 0.5617879f, 0.4712365f, 0.1511751f, -0.2206744f, -0.3499003f, -0.6380556f, -0.5280544f, -0.3092738f, -0.02274995f, 0.3947291f, 0.4233998f, 0.2629154f, -0.2515558f, -0.4678986f, -0.7101985f, -0.6812729f, -0.7570518f, -0.6256268f, 0.2984953f, 0.6755913f, 0.04583666f, -0.147064f, -0.2067336f, -0.3895372f, -0.370684f, -0.4781513f, -0.3436539f, 0.5610231f, 0.7379488f, 0.3025787f, -0.03146166f, -0.2143933f, -0.444064f, -0.3570024f, -0.3627875f, -0.164752f, 0.5945189f, 0.6534933f, 0.001179449f, -0.2756702f, -0.379542f, -0.5701987f, -0.5952791f, -0.59172f, -0.5030983f, 0.3132345f, 0.6014471f, 0.3966045f, -0.06361859f, -0.3512373f, -0.6145571f, -0.4666934f, -0.3260039f, -0.0241107f, 0.3408079f, 0.5668043f, 0.2366369f, -0.1372455f, -0.2685744f, -0.5398306f, -0.2969313f, -0.3100694f, -0.07913374f, 0.4873251f, 0.4850225f, 0.1457689f, -0.4269055f, -0.4452905f, -0.5796234f, -0.2518008f, -0.4196718f, 0.03067135f, 0.6036471f, 0.5112324f, 0.5218174f, -0.004717804f, -0.3786915f, -0.7267107f, -0.4286155f, -0.2862751f, -0.1132011f, 0.4760763f, 0.2979668f, 0.2093676f, -0.269059f, -0.4658153f, -0.7805324f, -0.3797049f, -0.4780289f, 0.145105f, 0.3760992f, 0.5229653f, -0.007027484f, -0.3944061f, -0.3962356f, -0.5645859f, -0.3131487f, -0.3893565f, 0.08796689f, 0.7142172f, 0.6631506f, -0.0225877f, -0.3807571f, -0.4513573f, -0.699247f, -0.4728136f, -0.4045135f, 0.1849866f, 0.6871158f, 0.3621373f, -0.08861838f, -0.3209723f, -0.1887773f, -0.575135f, -0.206488f, -0.2737719f, 0.4165217f, 0.4920602f, 0.5123313f, 0.2843927f, -0.1013372f, -0.09477463f, -0.6045378f, -0.3146276f, -0.1410113f, 0.3351844f, 0.5765313f, 0.640798f, -0.06413086f, -0.2922907f, -0.3439581f, -0.6772947f, -0.4745686f, -0.4342965f, -0.02908512f, 0.6393023f, 0.5333622f, -0.04836477f, -0.3134374f, -0.3071027f, -0.7975281f, -0.4198452f, -0.4683002f, -0.1388281f, 0.4634895f, 0.5703626f, 0.3211884f, 0.02763642f, -0.009560863f, -0.6167247f, -0.4298522f, -0.1702563f, 0.01550598f, 0.5348756f, 0.5177138f, 0.08446617f, -0.1679178f, -0.02459663f, -0.7955815f, -0.1919706f, -0.05805434f, 0.2027932f, 0.4988614f, 0.7011751f, -0.003586233f, -0.3131829f, -0.2141967f, -0.7260189f, -0.31204f, -0.4173874f, -0.104295f, 0.6011427f, 0.6489641f, -0.1654063f, -0.5744054f, -0.3321111f, -0.8149267f, -0.2653521f, -0.4066457f, -0.1692873f, 0.7622387f, 0.6123263f, 0.09723014f, -0.1619026f, 0.03205296f, -0.623102f, -0.0694637f, -0.3073372f, 0.04750448f, 0.7490851f, 0.5982275f, -0.1894663f, -0.4374092f, -0.2010498f, -0.8432049f, -0.2953232f, -0.389704f, -0.2515745f, 0.6142534f, 0.788205f, 0.02962166f, -0.5372174f, -0.3570836f, -0.7988299f, -0.2325774f, -0.2347484f, -0.04675489f, 0.8554967f, 0.6060407f, 0.2229074f, -0.2224571f, -0.2008772f, -0.6559119f, -0.4251911f, -0.4754379f, -0.1474938f, 0.4465253f, 0.7516544f, -0.1416059f, -0.4769597f, -0.3735782f, -0.9434796f, -0.2135582f, -0.1553396f, -0.0265525f, 0.6377608f, 0.6414058f, 0.1369849f, 0.02191103f, -0.2068602f, -0.6355029f, -0.03158649f, -0.4417929f, 0.008548528f, 0.4684504f, 0.3610998f, 0.2656096f, -0.004510282f, 0.2355661f, 0.09396905f, 0.1227658f, -0.2878832f, -0.1686379f, -0.0166293f, 0.2282291f, -0.03308947f, -0.2237171f, 0.105579f, 0.1329879f, 0.3230053f, 0.0005296469f, -0.08756581f, 0.05470502f, 0.7023488f, 0.2531722f, 0.1601535f, 0.1605094f, -0.3135025f, -0.04718936f, -0.4352519f, -0.006383792f, 0.504904f, 0.5791993f, -0.02240941f, -0.2984695f, -0.1319215f, -0.8504682f, -0.3147826f, -0.1456741f, 0.1002229f, 0.4199102f, 0.2400831f, -0.139903f, -0.2221355f, -0.2414577f, -0.3022395f, -0.1177127f, -0.07914025f, 0.005808562f, 0.1882947f, 0.4199462f, -0.1786929f, -0.3737127f, -0.3435594f, -0.501175f, -0.3037567f, -0.2601824f, -0.06821308f, 0.3389299f, 0.7196308f, -0.1502716f, -0.07291168f, -0.2914947f, -0.8255846f, -0.2531509f, -0.1621454f, 0.0004986785f, 0.7720796f, 0.5638187f, -0.1869945f, -0.2661841f, -0.3003312f, -0.6498221f, 0.3094784f, 0.08825556f, 0.2093408f, 0.6670656f, 0.2286424f, -0.09753937f, -0.2811871f, -0.368237f, -0.3894387f, -0.2070527f, -0.249191f, -0.1214703f, 0.1375646f, 0.06262719f, -0.1606559f, -0.3171239f, -0.4275349f, -0.6313657f, -0.3010728f, -0.3402846f, -0.3194602f, 0.02712896f, 0.7988951f, -0.0697612f, -0.09286097f, -0.2161744f, -0.4791192f, 0.235309f, 0.2271598f, 0.5534149f, 0.9118643f, 0.6118756f, 0.1767804f, -0.2163977f, -0.4413928f, -0.8086251f, -0.3340994f, -0.333468f, 0.169157f, 0.7084486f, 0.1194781f, -0.06289562f, -0.1297235f, -0.1976291f, -0.5048668f, -0.3492289f, -0.3670104f, -0.2680615f, 0.01408063f, 0.1483671f, -0.007450003f, -0.1297305f, -0.2972585f, -0.571654f, -0.5660949f, -0.5913292f, -0.2836451f, -0.1593389f, 1.015539f, 0.3348058f, -0.1694588f, -0.2631144f, -0.6122521f, -0.3785087f, -0.392275f, -0.1521694f, 0.9925659f, 0.838485f, 0.252406f, -0.1314205f, -0.3017025f, -0.8543435f, -0.579274f, -0.4618778f, -0.02844384f, 0.7349802f, 0.04345728f, -0.08692514f, -0.04503632f, -0.2258815f, -0.5700772f, -0.4686468f, -0.4413333f, -0.2509947f, -0.2280595f, 0.005880345f, -0.3498969f, -0.01153232f, 0.491067f, 0.3578552f, -0.1535028f, -0.1183453f, -0.1821463f, -0.1480073f, 0.6368828f, 0.07036841f, -0.1316347f, -0.3810168f, -0.7791548f, -0.4513743f, -0.3172011f, 0.03864256f, 0.6100842f, 0.7457109f, 0.1193339f, 0.07881083f, 0.01570004f, -0.04234686f, 0.2266035f, 0.2271062f, 0.3024233f, 0.7000731f, -0.1241089f, -0.3582299f, -0.2950243f, 0.3416787f, 0.4163121f, 0.2141351f, 0.08795159f, -0.1353201f, -0.2489174f, -0.3480219f, -0.4805101f, -0.5733116f, -0.5509276f, -0.7237235f, -0.08613665f, -0.2052407f, -0.299417f, -0.3714543f, 0.7041873f, 0.4149085f, 0.3529659f, 0.5361974f, 0.4667057f, 0.7507992f, 0.4461642f, 0.4142677f, 0.6832408f, -0.1856041f, -0.2718117f, -0.2823906f, -0.04174127f, 0.001458347f, 0.2320925f, -0.309454f, -0.2339889f, -0.1671201f, -0.535463f, };
    }
}
