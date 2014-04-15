package eerCalc;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import sun.org.mozilla.javascript.tools.shell.Environment;


public class EERCalculator {

	
	File _in;
	File _appendToThisFile;
	String _labelInTarget;
	
	double[] _course_matches;
	double[] _course_mismatches;
	
	int _trueMatchCnt;
	int _trueRejectCnt;
	
	double[] _fine_m;
	double[] _fine_mismatches;
	
	private class DistWithMatchInfo
	{
		public boolean isMatch;
		public double dist;
		
		public DistWithMatchInfo(double Distance, boolean isAMatch)
		{
			isMatch = isAMatch;
			dist = Distance;
		}
	}
	
	private static final int N=1000;
	private static final int THRESHS_TO_TRY = 100;
	private static final double SEARCH_INTERVAL_PERC_AROUND_COURSE_THRES=0.2; // searches +- 2.5% around the course thres
	
	private static final String SEPARATOR=",";
	private static final String FNAME_SEPARATOR="_";
	private static final double COURSE_LIMIT = 1.0;
	private static final double COURSE_STEP = COURSE_LIMIT / N;
	
	private ArrayList<DistWithMatchInfo> _matchesWithDist;
	
	
	public EERCalculator(File input, String caption, File fileToAppendTo)
	{
		_in = input;
		_appendToThisFile = fileToAppendTo;
		_labelInTarget = caption;
	}
	
	
	private boolean isEyeA(String fName)
	{
		return fName.split("_")[0].split("-")[1].equals("A");
	}
	
	private int getSampleNr(String fName)
	{
		
		String[] parts = fName.split("_");
		parts = parts[1].split("\\.");
		
		return Integer.parseInt(parts[0]);
	}
	
	private boolean sameClass(String f1, String f2)
	{
		boolean isMatch = f1.split(FNAME_SEPARATOR)[0].equals(f2.split(FNAME_SEPARATOR)[0]);
		return isMatch;
	}
	
	
	public void calc() throws IOException
	{
		// Determines the location 
		_course_matches = new double[N];
		_course_mismatches = new double[N];
		
		
		_trueMatchCnt = _trueRejectCnt = 0; // reset counters
		
		BufferedReader reader = new BufferedReader(new FileReader(_in));
		
		String line = null;
		_matchesWithDist = new ArrayList<DistWithMatchInfo>();
		
		reader.readLine(); // Skip header
		while((line = reader.readLine())!= null)
		{
			//System.out.println(line);
			String[] parts = line.split(SEPARATOR);
			
			// files
			String f1 = parts[0];
			String f2 = parts[1];
			
			
			// check eye
			// only consider Eye B
			//if((!isEyeA(f1) && !isEyeA(f2)))
				//continue;
			
			
			
			
			
			int idx = (int)(Double.parseDouble(parts[2]) / COURSE_STEP);
			
			// Clip at the top and bottom... seldom events anyway ;)
			if(idx > (N-1))
			{
				idx = N-1;
			}
			
			if(idx<0)
			{
				idx = 0;
			}
			
			// correct for self matches
			if(f1.equals(f2))
			{
				continue;
			}
			
			boolean isMatch = sameClass(f1, f2);
			if(isMatch)
			{
				_trueMatchCnt++;
				_course_matches[idx]++;
			}
			else
			{
				
				// Code to decrease number of postors
				int[] allowedSamples = {1,6};
				// only consider one image from right and left eye
				// but just in case of mismatch
				
			
				boolean isValid = false;
				
				for(int i:allowedSamples)
				{
					isValid = isValid || (getSampleNr(f1)==i);
					isValid = isValid || (getSampleNr(f2)==i);
				}
				
				isValid = isValid && ((!isEyeA(f1) && !isEyeA(f2)));
				//isValid = true;
				if(!isValid)
				{
					continue;
				}
				
				
				
				
				_trueRejectCnt++;
				_course_mismatches[idx]++;
			}
			
			
			_matchesWithDist.add(new DistWithMatchInfo(Double.parseDouble(parts[2]), isMatch));
		}
					
		reader.close();
		System.out.println("Data in memory...");
		System.out.println("Genuine: " + _trueMatchCnt + " and Postors: " + _trueRejectCnt);
		
		
		// normalize course
		
		double matchFactor = 1.0/(double)_trueMatchCnt;
		double mismatchFactor = 1.0 / (double) _trueRejectCnt;
		
		for(int i=0; i<N; i++)
		{
			_course_matches[i] *= matchFactor;
			_course_mismatches[i] *= mismatchFactor;
		}
		
		
		
		// find course EER
		
		double minDist = 1000.0;
		int courseThres_idx = -1;
				
		// integrate over it... it is safe to assume that either the half of matches or mismatches are to 
		// the left of the threshold...
		
		double integralMatches = 0.0;
		double integralMismatches = 0.0;
		double sidinessThres = 0.25;
				
		for(int i=0; i<N; i++)
		{
			
			integralMatches += _course_matches[i];
			integralMismatches += _course_mismatches[i];
			
			double dist = Math.abs(_course_matches[i] - _course_mismatches[i]);
			
			if(integralMatches < sidinessThres && integralMismatches < sidinessThres)
			{
				continue;
			}
			
			if (integralMatches > sidinessThres && integralMismatches > sidinessThres)
			{
				break;
			}
			
			
			if(dist < minDist)
			{
				courseThres_idx = i;
				minDist = dist;
			}			
		}
		// TODO: Test if matches are left or right... necessary?	
			
		BufferedWriter bw = new BufferedWriter(new FileWriter("output.csv"));		
		bw.append("distance, matches, mismatches" + "\n");
		
		for(int i=0; i<N; i++)
		{
			bw.append(i*COURSE_STEP + ", "+ _course_matches[i] + ", " + _course_mismatches[i] + "\n");
		}
		
		bw.flush();
		bw.close();
		
		
		System.out.println("Course EER is located at " + courseThres_idx*COURSE_STEP);
		System.out.println("Now computing the EER for Thresholds around ");
		
		double courseThres = courseThres_idx*COURSE_STEP;
		
		double fineStep = SEARCH_INTERVAL_PERC_AROUND_COURSE_THRES / THRESHS_TO_TRY;
		
		double[] far = new double[THRESHS_TO_TRY];
		double[] frr = new double[THRESHS_TO_TRY];
		
		
		for(int i=0; i<THRESHS_TO_TRY; i++)
		{
			double thres = courseThres - SEARCH_INTERVAL_PERC_AROUND_COURSE_THRES/2 + i*fineStep;
			
			for(DistWithMatchInfo d:_matchesWithDist)
			{
				if(d.isMatch)
				{
					if(d.dist > thres)
					{
						frr[i]++;
					}
				}
				else {
					if(d.dist <= thres)
					{
						far[i]++;
					}
				}
			}
			
			// normalize
			far[i] /= _trueRejectCnt;
			frr[i] /= _trueMatchCnt;
		}
		
		// OK... finally find the EER
		
		minDist = 100.0;
		int eerIDX = -1;
		
		bw = new BufferedWriter(new FileWriter("rates.csv"));		
		bw.append("thres, far, frr" + "\n");
		
		for(int i=0; i<THRESHS_TO_TRY; i++)
		{
			double dist = Math.abs(far[i]-frr[i]);
			
			if(dist < minDist)
			{
				minDist = dist;
				eerIDX = i;
			}
			
			bw.append(courseThres - SEARCH_INTERVAL_PERC_AROUND_COURSE_THRES/2 + i*fineStep + ", "+ far[i] + ", " + frr[i] + "\n");
		}
		
		bw.flush();
		bw.close();
		
		double eer = (far[eerIDX] + frr[eerIDX])/2.0;
		double tau = (courseThres - SEARCH_INTERVAL_PERC_AROUND_COURSE_THRES/2.0 + eerIDX*fineStep);
		System.out.println("Done =) EER is with thres=" +  tau );
		System.out.println("EER is " + eer);
		System.out.println("Appending to " + _appendToThisFile);
		
		FileWriter fileWritter = new FileWriter(_appendToThisFile,true);
        BufferedWriter bufferWritter = new BufferedWriter(fileWritter);
        bufferWritter.append(_labelInTarget + SEPARATOR + tau + SEPARATOR + eer + "\n");
        bufferWritter.close();
		
		
	}
	
	
	public static void main(String[] args) 
	{
		
		if(args.length < 3)
		{
			System.err.print("Call with <inputfile> <labelIn> <thisFileToAppend>");
			return;
		}
		
		System.out.println(args[0]);
		
		// TODO Auto-generated method stub
		EERCalculator e = new EERCalculator(new File(args[0]), args[1], new File(args[2]));
		
		try {
			e.calc();
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
	}

}
