/*

Copyright 2004 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2004 team rUNSWift RoboCup entry.  You may  
redistribute it and/or modify it under the terms of the GNU General  
Public License as published by the Free Software Foundation; either  
version 2 of the License, or (at your option) any later version as  
modified below.  As the original licensors, we add the following  
conditions to that license:

In paragraph 2.b), the phrase "distribute or publish" should be  
interpreted to include entry into a competition, and hence the source  
of any derived work entered into a competition must be made available  
to all parties involved in that competition under the terms of this  
license.

In addition, if the authors of a derived work publish any conference  
proceedings, journal articles or other academic papers describing that  
derived work, then appropriate academic citations to the original work  
must be included in that publication.

This rUNSWift source is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
General Public License for more details.

You should have received a copy of the GNU General Public License along  
with this source code; if not, write to the Free Software Foundation,  
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

import java.io.*;
import java.util.*;
import java.lang.*;

public class getDogShifts
{
    static final int WIDTH = 208;
    static final int HEIGHT = 159;
	static final int COLOURS = 7;
	static final int COLOUR_ELEMENTS = 3;
	static final int DOGS = 12;
	static final int SAMPLES = 1000;
    
    static final int HALF_HEIGHT = (int)Math.floor(HEIGHT / 2);
    static final int HALF_WIDTH = (int)Math.floor(WIDTH / 2);
    
	static final int SAMPLE_HEIGHT_FROM = HALF_HEIGHT - 5;
	static final int SAMPLE_HEIGHT_TO = HALF_HEIGHT + 5;
	static final int SAMPLE_WIDTH_FROM = HALF_WIDTH - 5;
    static final int SAMPLE_WIDTH_TO = HALF_WIDTH + 5;


    protected static byte y[] = new byte[WIDTH];
    protected static byte u[] = new byte[WIDTH];
    protected static byte v[] = new byte[WIDTH];
    protected static byte rest[] = new byte[4*WIDTH];


    public static String usage = 
     "Usage: java getDogShifts <image_directory> [base_dog]\n" 
    +"Help: java getDogShifts --help";
    
    public static String help = 
     "\ngetDogShifts\n"
    +"------------------------------\n"
    +"this program will process a group of calibration images\n"
    +"and for each Y, U and V of every dog it will return the function\n"
    +"to transform the value so that it matches that of the base dog's.\n"
    +"\n"
    +"Step 1)\n"
    +"for each of the 8 dogs take 10 pictures in BFL format (11/3/05) of\n" 
    +"EACH of the following 7 colours (i.e. 560 pictures all up);\n"
    +"yellow, green, blue, red, lightblue, white, pink\n"
    +"the pictures should be taken so the colour fills roughly the entire screen\n"
    +"each picture should be named as follows;\n" 
    +"<first letter of dog colour><jersey number>-<colour>-<abstract identifier>.BFL\n"
    +"for example r2-yellow-234.BFL , b3-lightblue-YUV32.BFL\n"
    +"IMPORTANT NOTE: these photos should be taken running code that does not\n"
    +"already shift the values, i.e. comment out all non default entries in\n"
    +"DogMacLookupType in VisualCortex.cc, recompile the dog and use that code\n"
    +"to take images\n"
    +"Note: you may now optionally include four white dogs\n"
    +"whose image file names should start with w1, w2, w3 & w4\n"
    +"\n"
    +"Step 2)\n"
    +"put all the pictures in a directory\n"
    +"run getDogShifts on that directory, note you can specify\n"
    +"a base dog in the form <first letter of dog colour><jersey number>\n" 
    +"however b1 will be taken as default if you do not\n"
    +"\n"
    +"Step 3)\n"
    +"in that directory there should now be a group of .csv files, these are designed\n"
    +"to be opened in OpenOffice which should have no trouble importing them to calc\n"
    +"you should set calc's default decimal places to 4 for more accurate results\n"
    +"for each Y, U and V of each of the 7 non base dogs there is a function of the\n"
    +"form y = m*x + b that maps the value to that of the base dog, in the .csv files\n"
    +"the value of 'm' is given by the '1/m' field and the value of 'b' is given by\n"
    +"the '-b/m' field\n"
    +"\n"
    +"Step 4)\n"
    +"input these values into the VisualCortex.cc file and make sure the current\n"
    +"colour calibration is for the base dog\n";
    
    public static int byte2UInt(byte b)
    {
		int i,s;

		s = (b >> 7) & 0x01; 		// save the sign bit
		b &= 0x7f; 			// strip the sign bit
		i = (((int) b) | (s<<7)); 	// reassemble number

		return i;
	}

	public static void main(String args[]){
		if (args.length < 1 || args.length > 2) {
			System.err.println(usage);
			return;
		}

        if (args[0].compareTo("--help") == 0){
            System.err.println(help);
            return;
        }
        
		int i,j,k,m;
        String path, baseDog;
        
		path = args[0];
		
		if (args.length > 1){baseDog = args[1];}
		else{baseDog = "b1";}
        
		File dir = new File(path);
		if (!dir.isDirectory()) {
			System.err.println(path+" is not a directory");
			return;
		}

		path += File.separator;

        
        String[] colourArray = 
            { "yellow","red","blue","green","pink","lightblue","white"};
        
        HashMap<String, Integer> colours = new HashMap<String, Integer>();
        //initialise colour map
        for(i = 0; i < COLOURS; i++) 
            colours.put(colourArray[i],new Integer(i));
        
        
        String[] dogArray = { "r1","r2","r3","r4","b1","b2","b3","b4","w1","w2","w3","w4"};
        int[] gotDogData =  {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0  };
        
        String[] dogKeys = new String[DOGS-1];
        
        HashMap<String, Integer> dogs = new HashMap<String, Integer>();
        int dogi = 0;
        for (i = 0; i < DOGS; i++){
            
            if (baseDog.compareTo(dogArray[i]) != 0){
                dogs.put(dogArray[i], new Integer(dogi));
                dogKeys[dogi] = dogArray[i];
                dogi++;
            }
        }        
        
		String[] fileList = dir.list();
        String name, colour, dog;
        int firstHyphen, secondHyphen;
        int t_dog, t_colour, t_sample;
        int lastcolour = -1;
        
        float baseDogVals[][] = new float[COLOURS][COLOUR_ELEMENTS];
        int baseDogValsCount[] = new int[COLOURS];
        int restDogVals[][][][] = new int[DOGS-1][COLOURS][COLOUR_ELEMENTS][SAMPLES];
        int sampleIndex[][] = new int[DOGS-1][COLOURS];
        
        //initialise baseDogValsCount array to zero
        for (i = 0; i < COLOURS; i++)
                baseDogValsCount[i] = 0;
        
        //initialise restDogVals to -1
        //so we can detect if less than SAMPLE values are taken
        for (i = 0; i < (DOGS-1); i++)
            for (j = 0; j < COLOURS; j++)
                for (k = 0; k < COLOUR_ELEMENTS; k++)
                    for (m = 0; m < SAMPLES; m++)
                        restDogVals[i][j][k][m] = -1;                
                
        //initialise sampleIndex array to zero
        for (i = 0; i < (DOGS-1); i++)
            for (j = 0; j < COLOURS; j++)
                    sampleIndex[i][j] = 0;
        
		try {
			FileInputStream in = null;
            
			for(k = 0; k < fileList.length; k++) {
				if(!fileList[k].toLowerCase().endsWith(".bfl"))
					continue;
					
				firstHyphen = fileList[k].indexOf('-');
				secondHyphen = fileList[k].indexOf('-', firstHyphen+1);
				colour = fileList[k].substring(firstHyphen + 1, secondHyphen);
				dog = fileList[k].substring(0,firstHyphen);
				
				name = path+fileList[k];
				in = new FileInputStream(name);
				if (lastcolour != colours.get(colour)){
                    System.out.println();
                    System.out.print("Reading "+dog+" "+colour);
                    lastcolour = colours.get(colour);
                }
                else System.out.print(".");

				for (i = 0; i < SAMPLE_HEIGHT_TO; i++) {
		
					in.read(y);
					in.read(u);
					in.read(v);
					in.read(rest);
                    
                    //discard data until we come to sample area
					if (i < SAMPLE_HEIGHT_FROM) continue;
					
					for (j = SAMPLE_WIDTH_FROM; j < SAMPLE_WIDTH_TO; j++) {
                    
						t_colour = (colours.get(colour)).intValue();

                        //if file is data for the base dog
						if (baseDog.compareTo(dog) == 0)
						{
							baseDogVals[t_colour][0] = 
                                baseDogVals[t_colour][0] + byte2UInt(y[j]);
                                
                            baseDogVals[t_colour][1] = 
                                baseDogVals[t_colour][1] + byte2UInt(u[j]);
                                
							baseDogVals[t_colour][2] = 
                                baseDogVals[t_colour][2] + byte2UInt(v[j]);

                                
							baseDogValsCount[t_colour] = 
                                baseDogValsCount[t_colour] + 1;
                               
						}
						else
						{

							t_dog = (dogs.get(dog)).intValue();     
                            gotDogData[t_dog] = 1;
                                                   
							t_sample = sampleIndex[t_dog][t_colour];

                            if (t_sample < SAMPLES){
    
                                restDogVals[t_dog][t_colour][0][t_sample] = 
                                    byte2UInt(y[j]);
                                    
						        restDogVals[t_dog][t_colour][1][t_sample] = 
                                    byte2UInt(u[j]);
                                    
						        restDogVals[t_dog][t_colour][2][t_sample] = 
                                    byte2UInt(v[j]);       
                            }
                                  
                            //increment sample index
                            sampleIndex[t_dog][t_colour] = 
                                sampleIndex[t_dog][t_colour] + 1;
						}
                        
					}
                    
				}
                
				in.close();    
			}    
			

                
                //calculate the average Y,U and V for each
                //colour for the base dog
                for (i = 0; i < COLOURS; i++){
                    
                    for (j = 0; j < COLOUR_ELEMENTS; j++){
                        baseDogVals[i][j] = 
                            baseDogVals[i][j] / baseDogValsCount[i];
                    }
                }
                        

                PrintWriter out = null;
                System.out.println();
                
                
                String[] LineEst = { "LINEST(A14:A7013;B14:B7013;1;1)",
                                     "LINEST(D14:D7013;E14:E7013;1;1)",
                                     "LINEST(G14:G7013;H14:H7013;1;1)"};
                
                
                String savefile;
                
                for (i = 0; i < (DOGS-1); i++){ 
                
                    if (gotDogData[i] == 0) {continue;}
               
                    savefile = path+"out-"+dogKeys[i]+"-withbase-"+baseDog+".csv";
                    out = new PrintWriter(new FileWriter(savefile),true);
                    System.out.println("Writing "+savefile);    
                    
                    out.println();
                    out.print("LINEST FOR Y,,,LINEST FOR U,,,LINEST FOR V,");
                    out.println();
                    
                    for (j = 1; j < 6; j++){
                    
                        out.print("=INDEX("+LineEst[0]+";"+String.valueOf(j)+";1),");
                        out.print("=INDEX("+LineEst[0]+";"+String.valueOf(j)+";2),,");
                        
                        out.print("=INDEX("+LineEst[1]+";"+String.valueOf(j)+";1),");
                        out.print("=INDEX("+LineEst[1]+";"+String.valueOf(j)+";2),,");
                                    
                        out.print("=INDEX("+LineEst[2]+";"+String.valueOf(j)+";1),");
                        out.print("=INDEX("+LineEst[2]+";"+String.valueOf(j)+";2),,");
                        
                        out.println();
                    }
                    
                    out.println();
                    
                    out.print("1/m,-b/m,,1/m,-b/m,,1/m,-b/m,,");
                    
                    out.println();
                    
                    
                    out.print("=1/A3,=-B3/A3,,=1/D3,=-E3/D3,,=1/G3,=-H3/G3,,");
                    out.println();
                    
                    out.println();
                    out.print("Y,,,");
                    out.print("U,,,");
                    out.print("V,");
                    out.println();               
                        
                    out.print(dogKeys[i]+","+baseDog+",,");
                    out.print(dogKeys[i]+","+baseDog+",,");
                    out.print(dogKeys[i]+","+baseDog+",,");              
                    out.println();      
                           
                    for(j = 0; j < COLOURS; j++){
                    
                        for (k = 0; k < SAMPLES; k++){
                            
                            if (restDogVals[i][j][0][k] == -1){
                                out.print(",,LIMITED SAMPLES,");
                                out.print(",,LIMITED SAMPLES,");
                                out.print(",,LIMITED SAMPLES,");
                            }
                            else
                            {
                                for (m = 0; m < COLOUR_ELEMENTS; m++){
                                    out.print(String.valueOf(restDogVals[i][j][m][k])+",");
                                    out.print(String.valueOf(baseDogVals[j][m])+",");
                                    out.print(colourArray[j]+",");
                                }
                            }
                            out.println();
                            
                            
                        }
                    }
                    
                    out.println();
                    out.print("Analysis");
                    out.println();
                    
                    String a;
                    String b;
                    String base;
                    String avg;
                    String lin;

                    out.println();
                    out.print("Y,base val,dog avg,*diff*,lin shift,*diff*,linD < avgD");
                    out.println();
                    
                    for(j = 0; j < COLOURS; j++){
                        
                        a = String.valueOf((j*1000)+14);
                        b = String.valueOf(((j+1)*1000)+13); 
                        base = "B"+a;
                        avg = "AVERAGE(A"+a+":A"+b+")";
                        lin = "((A"+b+"*A10)+B10)";
                    
                        out.print(colourArray[j]+",");
                        out.print("=" + base + ",");
                        out.print("=" + avg + ",");
                        out.print("=ABS((" + base + ")-(" + avg + ")),");
                        out.print("=" + lin + ",");
                        out.print("=ABS((" + base + ")-(" + lin + ")),");
                        
                        a = String.valueOf((COLOURS*1000)+18+j);
                        out.print("=F"+a+"<D"+a);
                        
                        out.println();
                    }
                    
                    out.println();
                    out.println();
                    
                    out.println();
                    out.print("U,base val,dog avg,*diff*,lin shift,*diff*,linD < avgD");
                    out.println();
                    
                    for(j = 0; j < COLOURS; j++){
                        
                        a = String.valueOf((j*1000)+14);
                        b = String.valueOf(((j+1)*1000)+13); 
                        base = "E"+a;
                        avg = "AVERAGE(D"+a+":D"+b+")";
                        lin = "((D"+b+"*D10)+E10)";
                    
                        out.print(colourArray[j]+",");
                        out.print("=" + base + ",");
                        out.print("=" + avg + ",");
                        out.print("=ABS((" + base + ")-(" + avg + ")),");
                        out.print("=" + lin + ",");
                        out.print("=ABS((" + base + ")-(" + lin + ")),");
                        
                        a = String.valueOf((COLOURS*1000)+29+j);
                        out.print("=F"+a+"<D"+a);                        
                        
                        out.println();
                    }
                    
                    out.println();
                    out.println();                    
                    
                    out.println();
                    out.print("V,base val,dog avg,*diff*,lin shift,*diff*,linD < avgD");
                    out.println();
                    
                    for(j = 0; j < COLOURS; j++){
                        
                        a = String.valueOf((j*1000)+14);
                        b = String.valueOf(((j+1)*1000)+13); 
                        base = "H"+a;
                        avg = "AVERAGE(G"+a+":G"+b+")";
                        lin = "((G"+b+"*G10)+H10)";
                    
                        out.print(colourArray[j]+",");
                        out.print("=" + base + ",");
                        out.print("=" + avg + ",");
                        out.print("=ABS((" + base + ")-(" + avg + ")),");
                        out.print("=" + lin + ",");
                        out.print("=ABS((" + base + ")-(" + lin + ")),");
                        
                        a = String.valueOf((COLOURS*1000)+40+j);
                        out.print("=F"+a+"<D"+a);                           
                        
                        out.println();
                    }
                    
                    out.println();
                    
                    out.print(",,,Total Diff,,Total Diff");
                    
                    out.println();
                    
                    out.print(",,,=SUM(D7018:D7046),,=SUM(F7018:F7046),=SUM(G7018:G7046),");
                    out.print("of 21");
                            
                    out.println();
                    
                    
                                        
                    out.close();
                }
                        


		} catch (Exception e) {
			System.out.println(e);
            e.printStackTrace();
		}
    }
}
