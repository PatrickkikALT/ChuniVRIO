using System;
using System.Runtime.InteropServices;
using System.Text;
using UnityEditor.Rendering;
using UnityEngine;

public class ChuniIO : MonoBehaviour {
  // [SerializeField] private string shmemName = "/chuniio";
  // [SerializeField] private int size = 1024;

  [DllImport("chuniio.dll")]
  private static extern void send_button_temp(uint cell, bool value);

  [DllImport("chuniio.dll")]
  private static extern void chuni_io_slider_stop(); 
  
  // private int _readWrite = 2;
  // private int _protectionWrite = 2;
  // private int _mapShared = 1;

  public static ChuniIO Instance;

  private void Start() {
    if (Instance == null) Instance = this; else Destroy(this);
  }

  public static void SendBtnToIO(int cell) {
    send_button_temp((uint)cell, true);
  }

  public static void ReleaseBtnFromIO(int cell) {
    send_button_temp((uint)cell, false);
  }

  public void OnDestroy() {
    chuni_io_slider_stop();
  }
}