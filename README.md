# DeskSignal

Unreal Engine 5.8 project. A desk pawn polls an HTTP endpoint and exposes the latest direction as blueprint-readable flags.

The editor and game open `Content/Maps/Room`.

## Requirements

- Unreal Engine 5.8

Open `DeskSignal.uproject`. The engine generates IDE project files when you build from the editor.

## Desk pawn

`ADeskPawn` is a `ADefaultPawn` with the default move axes removed. Set `Url` on the placed instance, then call `Poll` from a blueprint.

`Poll` sends one `GET` and ignores further calls until that request finishes. The timeout is 1 second. An empty URL, a failed request, a non-200 response, or a body that is not the JSON below clears `Fresh` and leaves the direction flags unchanged.

A successful response sets:

| Property | JSON field |
| --- | --- |
| `Forward` | `forward` |
| `Back` | `back` |
| `Left` | `left` |
| `Right` | `right` |
| `Fresh` | `fresh` |

Every field must be a boolean.

```json
{
  "forward": false,
  "back": true,
  "left": false,
  "right": false,
  "fresh": true
}
```

`Content/Blueprints/BP_DeskPawn` is the pawn blueprint. `Content/UI/WBP_Signal` is the signal widget. `Content/Blueprints/BP_DeskGameMode` is the game mode.
