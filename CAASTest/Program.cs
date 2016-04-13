using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Emgu.CV;

using CAAS;

namespace CAASTest
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                System.Console.WriteLine("Usage: CAASTest <Image File>");
                return;
            }

            string filepath = args[0];
            Mat img = CvInvoke.Imread(args[0], Emgu.CV.CvEnum.LoadImageType.Color);

            CAAS_Result result = CAAS_CLR4_Tx.Detect(img);
        }
    }
}
