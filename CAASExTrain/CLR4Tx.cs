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


        public static void CropObjects()
        {
            List<CLR4TxSample> samples = LoadSamples(@"\users\jie\projects\Intel\data\CLR4-Tx\Original");
            for (int i = 0; i < samples.Count; i++)
            {
                CLR4TxSample sample = samples[i];

                Emgu.CV.Image<Gray, byte> image = new Emgu.CV.Image<Gray, byte>(sample.imageFile);

                Rectangle target_roi = new Rectangle((int)(sample.targetX + 0.5), (int)(sample.targetY + 0.5), (int)(sample.targetW + 0.5), (int)(sample.targetH + 0.5));
                image.ROI = target_roi;
                string target_cropped_file = sample.imageFile.Replace("original", "TargetCropped");
                image.Save(target_cropped_file);

                Rectangle isolator_roi = new Rectangle((int)(sample.isolatorX + 0.5), (int)(sample.isolatorY + 0.5), (int)(sample.isolatorW + 0.5), (int)(sample.isolatorH + 0.5));
                image.ROI = isolator_roi;
                string isolator_cropped_file = sample.imageFile.Replace("original", "IsolatorCropped");
                image.Save(isolator_cropped_file);
            }
        }
    }
}
