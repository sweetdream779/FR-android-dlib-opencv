/* Copyright 2016 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

package com.android.lpr;

import android.graphics.Bitmap;
import android.graphics.RectF;

import com.openalpr.jni.Alpr;
import com.openalpr.jni.AlprCoordinate;
import com.openalpr.jni.AlprPlate;
import com.openalpr.jni.AlprPlateResult;
import com.openalpr.jni.AlprResults;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import com.android.lpr.env.Logger;

public class OpenALPRDetector {
  private static final Logger LOGGER = new Logger();

  private float[] outputLocations;
  private float[] outputScores;
  private byte[] byteValues = null;

  private Alpr alpr;
  private boolean logStats = false;
  private String resultLine = "";


  public static OpenALPRDetector create(String runtimeDataDir, String openAlprConfFile, String country) throws IOException {
    final OpenALPRDetector d = new OpenALPRDetector();
    d.alpr = new Alpr(country, openAlprConfFile, runtimeDataDir);
    d.alpr.setTopN(1);
    d.alpr.setDefaultRegion("");
    return d;
  }

  private OpenALPRDetector() {}

  public List<Recognition> recognizeImage(final Bitmap bitmap) {
    final ArrayList<Recognition> recognitions = new ArrayList<Recognition>();
    //String line = "";
    if(byteValues == null) {
      byteValues = new byte[bitmap.getWidth() * bitmap.getHeight() * 3];
    }

    byteValues = getBytesFromBitmap(bitmap);
    try {
      AlprResults results = alpr.recognize(byteValues);

      /*resultLine = "OpenALPR Version: " + alpr.getVersion() +
              " Processing Time: " + results.getTotalProcessingTimeMs() + " ms";*/

      LOGGER.d("OpenALPR Version: " + alpr.getVersion());
      LOGGER.d("Image Size: " + results.getImgWidth() + "x" + results.getImgHeight());
      LOGGER.d("Processing Time: " + results.getTotalProcessingTimeMs() + " ms");
      LOGGER.d("Found " + results.getPlates().size() + " results");

      LOGGER.d("  %-15s%-8s\n", "Plate Number", "Confidence");
      int i = 0;
      //List<AlprRegionOfInterest> rois = results.getRegionsOfInterest();

      /*List<AlprRegionOfInterest> plateRegions = results.getPlateRegions();
      for(AlprRegionOfInterest plateRegion : plateRegions)
      {
        final RectF detection =
                new RectF(
                        plateRegion.getX(),
                        plateRegion.getY(),
                        plateRegion.getX() + plateRegion.getWidth(),
                        plateRegion.getY() + plateRegion.getHeight());
        Recognition recognition = new Recognition("" + i, "", .6f, detection);
        recognitions.add(recognition);
        LOGGER.d("===PlateRegion: %d, %d, %d, %d\n", plateRegion.getX(), plateRegion.getY(), plateRegion.getWidth(), plateRegion.getHeight());
      }*/


      /*if(results.getPlates().size() == 0)
        resultLine = resultLine + " Nothing was found.";*/

      for (AlprPlateResult result : results.getPlates())
        {
            AlprPlate bestPlate = result.getBestPlate();
            List<AlprCoordinate> coords = result.getPlatePoints();

            for(AlprCoordinate coord : coords)
              LOGGER.d("    coord x: %d, y: %d", coord.getX(), coord.getY());

            final RectF detection =
                  new RectF(
                          coords.get(0).getX(),
                          coords.get(0).getY(),
                          coords.get(2).getX(),
                          coords.get(2).getY());
            Recognition recognition = new Recognition("" + i, bestPlate.getCharacters(), bestPlate.getOverallConfidence(), detection);
            recognitions.add(recognition);
            LOGGER.d("%-15s%-2f\n", bestPlate.getCharacters(), bestPlate.getOverallConfidence());
            //resultLine = resultLine + " Found : " + bestPlate.getCharacters();
            //LOGGER.d("Roi: %d, %d, %d, %d\n", rois.get(i).getX(), rois.get(i).getY(), rois.get(i).getWidth(), rois.get(i).getHeight());
            i += 1;
        }
    }catch (final Exception e) {
      LOGGER.e(e, "Exception!");
      //resultLine = resultLine + " Error in detecting and recognizing a plate.";
      return recognitions;
    }

    return recognitions;
  }

  public String getVersion(){
    return alpr.getVersion();
  }

  public byte[] getBytesFromBitmap(Bitmap bitmap) {
    ByteArrayOutputStream stream = new ByteArrayOutputStream();
    bitmap.compress(Bitmap.CompressFormat.JPEG, 100, stream);
    return stream.toByteArray();
  }

    public void close() {
        alpr.unload();
    }

  public String getResultInformation(){
    return resultLine;
  }

  public void enableStatLogging(final boolean logStats) {
    this.logStats = logStats;
  }

}
