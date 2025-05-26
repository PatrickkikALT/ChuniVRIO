using System;
using System.Runtime.InteropServices;
using System.Text;
using UnityEditor.Rendering;
using UnityEngine;

public class ChuniIO : MonoBehaviour {

  [DllImport("chuniio.dll")]
  private static extern void chuni_io_send_button(uint btn);

  [DllImport("chuniio.dll")]
  private static extern void chuni_io_release_button(uint btn);

  [DllImport("chuniio.dll")]
  private static extern void chuni_io_slider_stop(); 
  
  public static ChuniIO Instance;

  private void Start() {
    if (Instance == null) Instance = this; else Destroy(this);
  }
  
  public static void SendBtnToIO(int cell) {
    chuni_io_send_button((uint)cell);
  }

  public static void ReleaseBtnFromIO(int cell) {
    chuni_io_release_button((uint)cell);
  }

  public void OnDestroy() {
    chuni_io_slider_stop();
  }
}