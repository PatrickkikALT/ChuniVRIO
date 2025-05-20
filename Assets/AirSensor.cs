using System.Runtime.InteropServices;
using UnityEngine;

public enum Direction {
  LEFT,
  RIGHT
}

public class AirSensor : MonoBehaviour {
  public Transform[] sensors;
  public Direction direction;
  public LayerMask playerLayer;
  public int air;
  [DllImport("chuniio.dll")]
  private static extern void send_air_temp(uint index, bool value);
  private void FixedUpdate() {
    if (direction == Direction.LEFT) {
      foreach (var sensor in sensors) {
        if (Physics.Raycast(sensor.position, -sensor.forward, out var hit, 20f)) {
          if (hit.transform.gameObject.layer == playerLayer.value) {
            send_air_temp((uint)air, true);
          }
          else {
            send_air_temp((uint)air, false);
          }
        }
      }
    }
    else {
      foreach (var sensor in sensors) {
        if (Physics.Raycast(sensor.position, -sensor.forward, out var hit, 20f)) {
          if (hit.transform.gameObject.layer == playerLayer.value) {
            send_air_temp((uint)air, true);
          }
          else {
            send_air_temp((uint)air, false);
          }
        }
      }
    }
  }
}
