public class HSLColorSelection {
  int drawn = 0;
  public Point position;
  public Size size;
  public Color rgb = new Color(255,255,255);
  public float[] hsl() { return RGB2HSL(rgb); };
  int drawWidth = 300;
  int drawHeight = 300;
  int vSteps() { return drawHeight/2; };
  int hSteps() { return drawWidth/2; }
  int barWidth() { return drawWidth/hSteps(); };
  int barHeight() { return drawHeight/vSteps(); };
  
  
  int cursorSize = 16;
  int cursorX = 0;
  int cursorY = 0;
  
  float saturation = 1.0;
    
  public HSLColorSelection(Point position, Size size)
  {
    this.drawWidth = size.width;
    this.drawHeight = size.height;
    this.position = position;
    this.size = size;
  }
   
  void draw()
  {
    if (drawn ==0) {
      background(0);
      for (int i = 0; i < hSteps(); i++)
      {
        noStroke();
        Color tmpColor = HSL2RGB(map(i * barWidth(), 0.0, drawWidth, 0.0, 360.0), saturation, .5);
        fill(tmpColor.rgb[0], tmpColor.rgb[1], tmpColor.rgb[2]);
        rect(this.position.x + barWidth() * i, this.position.y , barWidth(), drawHeight);
      }
      for (int i = 0; i <= vSteps()/2+1; i++)
      {
        noStroke();
        fill(0, 0, 0, (255/(vSteps()/2))*i);
        rect(this.position.x, this.position.y + (drawHeight/2) + barHeight() * i, drawWidth, barHeight());
      }
      for (int i = 0; i <= vSteps()/2+1; i++)
      {
        noStroke();
        fill(255, 255, 255, (255/(vSteps()/2))*i);
        rect(this.position.x, this.position.y + (drawHeight/2) - barHeight() * i, drawWidth, barHeight());
      }
      drawn = 0;
    }
    stroke(255,255,255,255);
    strokeWeight(2);
    fill(255,255,255,0);
    ellipse(cursorX, cursorY, cursorSize, cursorSize);
    
    fill(rgb.rgb[0], rgb.rgb[1], rgb.rgb[2]);
    noStroke();
    //rect(this.position.x, this.position.y + 190, width, 20);
  }
  
  void updateColor() {
    rgb = HSL2RGB(map(cursorX-this.position.x, 0.0, drawWidth, 0.0, 360.0), saturation, 1.0-((1.0/drawHeight)*(cursorY-this.position.y)));
    //println(rgb.rgb[0]);
    //println(this.hsl());
  }
  
  void updateSelectionFromRGB() {
    this.cursorX = int(map(this.hsl()[0], 0, 360, 0.0, drawWidth));
    this.cursorY = int(map(this.hsl()[2], 0.0, 1.0, drawHeight, 0));
    println(this.hsl()[0]);
    println("hue is ", this.hsl()[0]);
    println("setting cursor to ", cursorX);
    println("setting cursor to ", cursorY);
    //this.setSelection();
  }
  
  public void setSelection(int x, int y) {
    this.cursorX = x;
    this.cursorY = y;
    updateColor();
  }
  
  
  
  void mousePressed() {
    if (mouseX > 0 && mouseX < drawWidth &&
        mouseY > 0 && mouseY < drawHeight) {
      cursorX = mouseX;
      cursorY = mouseY;
      updateColor();
    }
  }
  
  void mouseDragged() {
    if (mouseX > 0 && mouseX < drawWidth &&
        mouseY > 0 && mouseY < drawHeight) {
      cursorX = mouseX;
      cursorY = mouseY;
      updateColor();
    }
    if (mouseX > 0 && mouseX < drawWidth &&
        mouseY > drawHeight && mouseY < height) {
      saturation = (1.0/width)*mouseX;
    }
  }
}