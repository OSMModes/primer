class Color {
  public int[] rgb;
  public String hex;
  public Point position;
  public Color(int red, int green, int blue) {
    rgb = new int[]{red,green,blue};
    hex = String.format("#%02x%02x%02x", red, green, blue);
  }
  public Color(int red, int green, int blue, Point position) {
    rgb = new int[]{red,green,blue};
    hex = String.format("#%02x%02x%02x", red, green, blue);
    this.position = position;
  }
}


class Point {
  public int x;
  public int y;
  public Point(int x, int y) {
    this.x = x;
    this.y = y;
  }
}

class Size {
  public int width;
  public int height;
  public Size(int width, int height) {
    this.width = width;
    this.height = height;
  }
}

class ColorButton {
  public Point position;
  public Size size;
  public Color Color;
  public ColorButton(Color theColor, boolean highlighted, int padding, Point position, Size size) {
    fill(theColor.rgb[0], theColor.rgb[1], theColor.rgb[2]);
    if (highlighted) {
      stroke(255,0,0);
    } else {
      stroke(0,0,0);
    }
    strokeWeight(padding);
    rect(position.x, position.y, size.width, size.height);
    this.position = position;
    this.size = size;
    this.Color = theColor;
  }
}


// Given H,S,L in range of 0-360, 0-1, 0-1  Returns a Color
Color HSL2RGB(float hue, float sat, float lum)
{
    float v;
    float red, green, blue;
    float m;
    float sv;
    int sextant;
    float fract, vsf, mid1, mid2;
 
    red = lum;   // default to gray
    green = lum;
    blue = lum;
    v = (lum <= 0.5) ? (lum * (1.0 + sat)) : (lum + sat - lum * sat);
    m = lum + lum - v;
    sv = (v - m) / v;
    hue /= 60.0;  //get into range 0..6
    sextant = floor(hue);  // int32 rounds up or down.
    fract = hue - sextant;
    vsf = v * sv * fract;
    mid1 = m + vsf;
    mid2 = v - vsf;
 
    if (v > 0)  
    {
        switch (sextant)
        {
            case 0: red = v; green = mid1; blue = m; break;
            case 1: red = mid2; green = v; blue = m; break;
            case 2: red = m; green = v; blue = mid1; break;
            case 3: red = m; green = mid2; blue = v; break;
            case 4: red = mid1; green = m; blue = v; break;
            case 5: red = v; green = m; blue = mid2; break;
        }
    }
    return new Color(int(red * 255), int(green * 255), int(blue * 255));
}

float[] RGB2HSL(Color rgb) {
  float r = rgb.rgb[0];
  float g = rgb.rgb[1];
  float b = rgb.rgb[2];
  r = r/255;
  g = g/255;
  b = b/255;
  float max = max(r,g,b);
  float min = min(r,g,b);
  float lum = (max + min)/2;
  float sat = 1.0;
  if (max == min) {
    sat = 0.0;
  } else if (lum <= 0.5) {
    sat = (max-min)/(max+min);
  } else {
    sat = (max-min)/(2.0-max-min);
  }
  float hue = 0.0;
  if (max == r) {
    hue = (g-b)/(max-min);
  } else if (max == g) {
    hue = 2.0 + (b-r)/(max-min);
  } else if (max == b) {
    hue = 4.0 + (r-g)/(max-min);
  }
  hue = hue * 60;
  if (hue < 0) {
    hue += 360;
  }
  return new float[] {hue, sat, lum};
}