// vim:set ts=4 sw=4 ai et:

import java.io.*;
import logic.Gesture;
import logic.Quantizer;

public class TestHarness {

    public static void main(String[] args) throws IOException {
        Gesture g = new Gesture();
        Quantizer q = new Quantizer(8);
        int[] results;

        BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
        String line;

        while ((line = stdin.readLine()) != null)
        {
            String[] field = line.split("[ \t]+");
            double[] point = new double[3];

            for (int i = 0; i < 3; i++)
                point[i] = Double.parseDouble(field[i]);

            System.out.println("Input coordinate " + point[0] + ", " + point[1] + ", " + point[2]);
            g.add(point);
        }

        q.trainCenteroids(g);
        results = q.getObservationSequence(g);

        for (int i = 0; i < results.length; i++) {
            System.out.printf("%d\n", results[i]);
        }

        return;
    }
}
