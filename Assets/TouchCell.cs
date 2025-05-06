using System;
using UnityEngine;

public class TouchCell : MonoBehaviour {
  [SerializeField] private int opBtn;
  private void OnCollisionEnter(Collision other) {
    
  }

  [ContextMenu("Test")]
  public void SendBtn() {
    ChuniIO.Instance.SendBtnToSharedMemory(opBtn);
  }
}
