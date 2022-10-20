import androidx.compose.material.MaterialTheme
import androidx.compose.desktop.ui.tooling.preview.Preview
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.height
import androidx.compose.material.Button
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Window
import androidx.compose.ui.window.application
import io.ktor.client.*
import io.ktor.client.plugins.websocket.*
import io.ktor.http.*
import io.ktor.websocket.Frame
import io.ktor.websocket.readText
import kotlinx.coroutines.delay

private enum class ConnectionState { UNKNOWN, CONNECTED, CONNECTING, ERROR }

private data class Connection(
  val host: String = "localhost",
  val port: Int = 8082,
  val path: String = "/",
  val state: MutableState<ConnectionState> = mutableStateOf(ConnectionState.UNKNOWN),
  val reconnectDelay: Int = 5,
)

@Composable
@Preview
fun App() {
  val connection = remember { Connection() }
  val client = remember { HttpClient { install(WebSockets) } }
  var message by remember { mutableStateOf("") }

  LaunchedEffect(Unit) {
    while (true) {
      connection.state.value = ConnectionState.CONNECTING
      try {
        client.webSocket(HttpMethod.Get, connection.host, connection.port, connection.path) {
          message = "Connected!"
          connection.state.value = ConnectionState.CONNECTED
          while (true) {
            val frame = incoming.receive() as? Frame.Text ?: continue
            message = frame.readText()
          }
        }
      } catch (e: Exception) {
        connection.state.value = ConnectionState.ERROR
        repeat(connection.reconnectDelay) {
          val remaining = connection.reconnectDelay - it
          message = "Connection interrupted, will re-connect in $remaining seconds..."
          delay(1000)
        }
      }
    }
  }

  DisposableEffect(Unit) { onDispose { client.close() } }

  MaterialTheme {
    when (connection.state.value) {
      ConnectionState.CONNECTED -> {
        Row(Modifier.height(40.dp)) {
          Text(modifier = Modifier.align(Alignment.CenterVertically),
               text = "Listening on ${connection.host}:${connection.port} ")
          Button(onClick = {}) { Text(message) }
        }
      }
      ConnectionState.ERROR -> Text(text = message, color = Color.Red)
      else -> Text(connection.state.value.name)
    }
  }
}

fun main() = application { Window(onCloseRequest = ::exitApplication) { App() } }
