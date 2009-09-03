// vim:set ts=4 sw=4 ai et:

import java.io.*;
import logic.*;

public class QuantizerTest {

    // Read coordinate data from stdin
    static void read_input(Gesture g) throws IOException {
        BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
        String line;

        while ((line = stdin.readLine()) != null)
        {
            String[] field = line.split("[ \t]+");
            double[] point = new double[3];

            for (int i = 0; i < 3; i++)
                point[i] = Double.parseDouble(field[i]);

            //System.out.println("Input coordinate " + point[0] + ", " + point[1] + ", " + point[2]);
            g.add(point);
        }
        return;
    }

    // Find the min and max values
    static void normalize_input(Gesture g) {
        double maxacc = Double.MIN_VALUE;
        double minacc = Double.MAX_VALUE;

        for (int i = 0; i < g.getData().size(); i++) {
            for (int coord = 0; coord < 3; coord++) {
                if (Math.abs(g.getData().elementAt(i)[coord]) > maxacc) maxacc = Math.abs(g.getData().elementAt(i)[coord]);
                if (Math.abs(g.getData().elementAt(i)[coord]) < minacc) minacc = Math.abs(g.getData().elementAt(i)[coord]);
            }
        }

        g.setMaxAcceleration(maxacc);
        g.setMinAcceleration(minacc);
        //System.out.printf("minacc %f, maxacc %f\n", minacc, maxacc);
        return;
    }

    public static void main(String[] args) throws IOException {
        Gesture g = new Gesture();
        Quantizer q = new Quantizer(8);
        int[] results;

        // Populate gesture data
        read_input(g);
        normalize_input(g);

        // Run through quantizer
        q.trainCenteroids(g);
        results = q.getObservationSequence(g);
        for (int i = 0; i < results.length; i++) {
            System.out.printf("%d\n", results[i]);
        }

        return;
    }
}
